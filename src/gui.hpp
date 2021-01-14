
#pragma once

#include "config.hpp"
#include "nic_mgr.hpp"

#include <cmath>
#include <cstring>
#include <functional>
#include <map>
#include <string>
#include <vector>

#include <X11/Xatom.h>
#include <X11/Xutil.h>
#include <poll.h>
#include <sys/timerfd.h>
#include <unistd.h>

namespace ashmon
{
class gui
{
    struct coord
    {
        int x{};
        int y{};
    };
    struct dimn
    {
        int w{};
        int h{};
    };
    struct button
    {
        std::string name;
        gui::coord coord{};
        gui::dimn dimn{};
        std::string text{};
        std::function<void()> on_click;
        bool disabled{};
        bool hovered{};
    };
    config* config_;
    nic_mgr* nic_mgr_;
    Display* display_;
    Window window_;
    std::map<std::string, GC> gcs_;
    std::vector<button> buttons_;
    std::array<pollfd, 2> poll_fds_{};
    bool window_extended_{};

  public:
    gui(config* config, nic_mgr* nic_mgr)
        : config_(config)
        , nic_mgr_(nic_mgr)
    {
        display_ = XOpenDisplay(nullptr);

        if (!display_)
            throw std::runtime_error{ "can't open display" };

        XVisualInfo vinfo{};
        if (!XMatchVisualInfo(display_, DefaultScreen(display_), 32, TrueColor, &vinfo))
            XMatchVisualInfo(display_, DefaultScreen(display_), 24, TrueColor, &vinfo);

        XSetWindowAttributes attr{};
        attr.colormap = XCreateColormap(display_, DefaultRootWindow(display_), vinfo.visual, AllocNone);
        attr.border_pixel = 0;
        attr.background_pixel = 0;
        window_ = XCreateWindow(
            display_,
            DefaultRootWindow(display_),
            0,
            0,
            180,
            90,
            0,
            vinfo.depth,
            InputOutput,
            vinfo.visual,
            CWColormap | CWBorderPixel | CWBackPixel,
            &attr);

        XStoreName(display_, window_, "ashmon");
        auto event_mask = ExposureMask | PointerMotionMask | LeaveWindowMask | EnterWindowMask | ButtonPressMask | ButtonReleaseMask;
        XSelectInput(display_, window_, event_mask);

        auto prop_type = XInternAtom(display_, "_NET_WM_WINDOW_TYPE", False);
        auto prop_data = XInternAtom(display_, "_NET_WM_WINDOW_TYPE_DOCK", False);
        XChangeProperty(display_, window_, prop_type, XA_ATOM, 32, PropModeReplace, reinterpret_cast<unsigned char*>(&prop_data), 1);

        const auto display_w = DisplayWidth(display_, DefaultScreen(display_));
        const auto display_h = DisplayHeight(display_, DefaultScreen(display_));
        const auto [window_x, window_y] = config_->window_xy();
        /* if window position is outside the screen (screen resolution have been changed) */
        if (window_x < 0 || window_x > display_w || window_y < 0 || window_y > display_h)
            XMoveWindow(display_, window_, display_w - 180 - 50, display_h - 90 - 50);
        else
            XMoveWindow(display_, window_, window_x, window_y);

        XMapWindow(display_, window_);

        buttons_.push_back({ "prev_nic", coord{ 1, 90 }, dimn{ 17, 17 }, "<", [&] { nic_mgr_->select_prev_nic(); } });
        buttons_.push_back({ "next_nic", coord{ 162, 90 }, dimn{ 17, 17 }, ">", [&] { nic_mgr_->select_next_nic(); } });

        update_gcs();

        /* setting timer_fd for 1 second clock */
        const auto timer_fd = timerfd_create(CLOCK_MONOTONIC, 0);
        itimerspec ts{ { 1, 0 }, { 1, 0 } };
        if (timerfd_settime(timer_fd, 0, &ts, nullptr) < 0)
        {
            close(timer_fd);
            throw std::runtime_error{ "timerfd_settime() failed: errno=" + std::string{ std::strerror(errno) } };
        }

        /* connection file descriptor for using in poll */
        const auto x11_fd = XConnectionNumber(display_);
        poll_fds_ = { { { x11_fd, POLLIN, 0 }, { timer_fd, POLLIN, 0 } } };
    }

    void run()
    {
        auto send_expose_event = false;
        auto window_moved = false;
        auto window_grab_x{ 0 };
        auto window_grab_y{ 0 };
        for (;;)
        {

            if (send_expose_event)
            {
                XEvent e{ Expose };
                XSendEvent(display_, window_, 0, ExposureMask, &e);
                send_expose_event = false;
            }

            if (!XPending(display_))
            {
                const auto polled = ::poll(poll_fds_.data(), poll_fds_.size(), -1);

                if (polled == -1 && errno != EINTR)
                    throw std::runtime_error("poll() failed: errno=" + std::string{ std::strerror(errno) });

                if (poll_fds_.at(1).revents) /* timerfd fired */
                {
                    uint64_t res{};
                    if (read(poll_fds_.at(1).fd, &res, sizeof(res)) < 0)
                        throw std::runtime_error("read() failed: errno=" + std::string{ std::strerror(errno) });
                    nic_mgr_->update();
                    send_expose_event = true;
                }
            }
            else
            {
                XEvent xevent;
                XNextEvent(display_, &xevent);
                switch (xevent.type)
                {
                    case Expose:
                    {
                        expose();
                        break;
                    }
                    case ButtonPress:
                    {
                        const auto e = xevent.xbutton;
                        if (e.button == Button1)
                        {
                            window_grab_x = e.x;
                            window_grab_y = e.y;
                        }
                        else if (e.button == Button3)
                        {
                            return; /* exit from the loop */
                        }
                        else if (e.button == Button4)
                        {
                            auto opacity = config_->opacity();
                            if (opacity <= 90)
                            {
                                config_->opacity(opacity + 10);
                                update_gcs();
                                send_expose_event = true;
                            }
                        }
                        else if (e.button == Button5)
                        {
                            auto opacity = config_->opacity();
                            if (opacity >= 30)
                            {
                                config_->opacity(opacity - 10);
                                update_gcs();
                                send_expose_event = true;
                            }
                        }
                        break;
                    }
                    case ButtonRelease:
                    {
                        const auto e = xevent.xbutton;
                        if (e.button == Button1)
                        {
                            if (window_moved)
                            {
                                config_->window_xy(e.x_root - window_grab_x, e.y_root - window_grab_y);
                                window_moved = false;
                            }
                            else
                            {
                                for (auto& btn : buttons_)
                                {
                                    if (btn.hovered && !btn.disabled)
                                    {
                                        btn.on_click();
                                        send_expose_event = true;
                                    }
                                }
                            }
                        }
                        break;
                    }
                    case MotionNotify:
                    {
                        const auto e = xevent.xmotion;
                        if (e.state & Button1Mask)
                        {
                            window_moved = true;
                            XMoveWindow(display_, window_, e.x_root - window_grab_x, e.y_root - window_grab_y);
                        }
                        for (auto& btn : buttons_)
                        {
                            if (e.x >= btn.coord.x && e.x < btn.coord.x + btn.dimn.w && e.y >= btn.coord.y &&
                                e.y < btn.coord.y + btn.dimn.h)
                            {
                                if (!btn.hovered)
                                {
                                    btn.hovered = true;
                                    send_expose_event = true;
                                }
                            }
                            else if (btn.hovered)
                            {
                                btn.hovered = false;
                                send_expose_event = true;
                            }
                        }
                        break;
                    }
                    case EnterNotify:
                    {
                        XResizeWindow(display_, window_, 180, 108);
                        window_extended_ = true;
                        break;
                    }
                    case LeaveNotify:
                    {
                        for (auto& btn : buttons_)
                        {
                            if (btn.hovered)
                            {
                                btn.hovered = false;
                                send_expose_event = true;
                            }
                        }
                        XResizeWindow(display_, window_, 180, 90);
                        window_extended_ = false;
                        break;
                    }
                }
            }
        }
    }

    ~gui()
    {
        for (const auto& gc : gcs_)
            XFreeGC(display_, gc.second);

        XCloseDisplay(display_);

        for (const auto& poll_fd : poll_fds_)
            close(poll_fd.fd);
    }

  private:
    void fill_rectangle(const std::string& gc_name, coord coord, dimn dimn)
    {
        XFillRectangle(display_, window_, gcs_.at(gc_name), coord.x, coord.y, dimn.w, dimn.h);
    }
    void draw_rectangle(const std::string& gc_name, coord coord, dimn dimn)
    {
        XDrawRectangle(display_, window_, gcs_.at(gc_name), coord.x, coord.y, dimn.w, dimn.h);
    }

    void draw_string(const std::string& gc_name, coord coord, const std::string& str)
    {
        XDrawString(display_, window_, gcs_.at(gc_name), coord.x, coord.y, str.data(), str.size());
    }

    void draw_string_center(const std::string& gc_name, coord coord, dimn dimn, std::string str)
    {
        auto extra = static_cast<int>(str.size()) - dimn.w / 6;
        if (extra > 0)
            str = str.substr(0, str.size() - extra - 2) + "..";

        auto x = coord.x + 1 + (dimn.w / 2 - str.size() * 6 / 2);
        auto y = coord.y + (dimn.h - 9) / 2 + 9;
        XDrawString(display_, window_, gcs_.at(gc_name), x, y, str.data(), str.size());
    }

    void draw_line(const std::string& gc_name, coord coord, dimn dimn)
    {
        XDrawLine(display_, window_, gcs_.at(gc_name), coord.x, coord.y, coord.x + dimn.w, coord.y - dimn.h);
    }

    void update_gcs()
    {
        struct gc_spec
        {
            std::string name;
            uint8_t r, g, b;
        };

        std::array<gc_spec, 8> gc_specs{ { { "rx", 0x22, 0x8B, 0x22 },
                                           { "tx", 0xDC, 0x14, 0x3C },
                                           { "rxtx", 0xFF, 0xA5, 0x00 },
                                           { "prime", 0x1A, 0x90, 0xFF },
                                           { "light", 0xBA, 0xDD, 0xFF },
                                           { "button", 0xF0, 0xF6, 0xFF },
                                           { "button_hover", 0xDA, 0xEC, 0xFF },
                                           { "background", 0xFF, 0xFF, 0xFF } } };

        for (const auto& gc_spec : gc_specs)
        {
            if (gcs_.find(gc_spec.name) == gcs_.end())
            {
                gcs_.emplace(gc_spec.name, XCreateGC(display_, window_, 0, nullptr));
                XSetLineAttributes(display_, gcs_.at(gc_spec.name), 1, LineSolid, CapNotLast, JoinMiter);
                const auto font = XLoadFont(display_, "6x13");
                XSetFont(display_, gcs_.at(gc_spec.name), font);
            }

            uint8_t alpha = config_->opacity() / 100.0 * 0xFF;
            uint32_t rgba = alpha << 24 | (gc_spec.r * alpha) / 255 << 16 | (gc_spec.g * alpha) / 255 << 8 | (gc_spec.b * alpha) / 255;
            XSetForeground(display_, gcs_.at(gc_spec.name), rgba);
        }
    }

    void expose()
    {
        const auto& nic = nic_mgr_->selected_nic();
        const auto max_rate = nic.max_rate();

        fill_rectangle("background", coord{ 0, 0 }, dimn{ 180, 123 });
        draw_rectangle("light", coord{ 0, 0 }, dimn{ 179, 122 });
        draw_line("light", coord{ 5, 55 }, dimn{ 170, 0 });
        draw_line("light", coord{ 1, 89 }, dimn{ 178, 0 });

        if (window_extended_)
        {
            draw_line("light", coord{ 1, 107 }, dimn{ 178, 0 });
            draw_line("light", coord{ 92, 125 }, dimn{ 0, 17 });

            for (auto& btn : buttons_)
            {
                if (btn.name == "prev_nic")
                    btn.disabled = !nic_mgr_->is_prev_nic_available();

                if (btn.name == "next_nic")
                    btn.disabled = !nic_mgr_->is_next_nic_available();

                std::string background_gc{ "button" };
                if (btn.hovered && !btn.disabled)
                    background_gc = "button_hover";
                fill_rectangle(background_gc, btn.coord, btn.dimn);

                std::string text_gc{ "prime" };
                if (btn.disabled)
                    text_gc = "light";
                draw_string_center(text_gc, btn.coord, btn.dimn, btn.text);
            }
            draw_string_center("prime", coord{ 18, 90 }, dimn{ 144, 17 }, nic_mgr_->selected_nic_name());
        }

        for (auto i = 1; i <= 3; i++)
            draw_string("prime", coord{ 5, i * 17 - 2 }, humanize_size(max_rate / i) + "/s");

        auto i = 0;
        for (const auto& rate : nic.rates())
        {
            if (max_rate == 0)
                break;

            const auto draw_candle = [&](auto gc_name, auto rate) {
                draw_line(gc_name, coord{ 75 + i, 55 }, dimn{ 0, static_cast<int>(rate * 50 / max_rate) });
            };

            if (rate.rx >= rate.tx)
                draw_candle("rx", rate.rx);
            else
                draw_candle("tx", rate.tx);

            draw_candle("rxtx", std::min(rate.rx, rate.tx));
            i++;
        }

        draw_string("rx", coord{ 5, 70 }, "D  " + humanize_size(nic.latest_rate().rx) + "/s");
        draw_string("tx", coord{ 92, 70 }, "U  " + humanize_size(nic.latest_rate().tx) + "/s");

        draw_string("prime", coord{ 5, 85 }, "TD " + humanize_size(nic.total_bytes().rx, 3));
        draw_string("prime", coord{ 92, 85 }, "TU " + humanize_size(nic.total_bytes().tx, 3));
    }

    static std::string humanize_size(uint64_t size, size_t max_precision = 2)
    {
        std::array<const char*, 7> units = { "B", "KB", "MB", "GB", "TB", "PB", "EB" };

        const auto str = std::to_string(size);

        if (str.size() <= 3)
            return { str + " " + units[0] };

        const auto order = static_cast<size_t>(std::ceil((str.size() - 3) / 3.0));

        auto int_digits = str.size() % 3;
        if (int_digits == 0)
            int_digits = 3;

        return { str.substr(0, int_digits) + "." + str.substr(int_digits, std::min(order, max_precision)) + " " + units[order] };
    }
};
} // namespace ashmon