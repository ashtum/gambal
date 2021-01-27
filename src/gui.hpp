
#pragma once

#include "config.hpp"
#include "proc.hpp"

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

namespace gambal
{
class gui
{
    struct coord
    {
        int x{};
        int y{};
        auto operator+(const coord& rhs) const
        {
            return coord{ x + rhs.x, y + rhs.y };
        }
    };
    struct dimn
    {
        int w{};
        int h{};
    };
    struct button
    {
        std::string name;
        std::string text{};
        std::function<void()> on_click;
        gui::coord coord{};
        gui::dimn dimn{};
        bool disabled{};
        bool hovered{};
    };
    config* config_;
    proc* proc_;
    Display* display_;
    Window window_;
    std::map<std::string, GC> gcs_;
    std::vector<button> buttons_;
    std::array<pollfd, 2> poll_fds_{};
    bool window_extended_{};
    struct style
    {
        std::string name;
        std::string font_name;
        Font font{};
        int thickness{};
        int char_w{};
        int char_h{};
        int window_width{};
    };
    std::vector<style> styles_{ { "normal", "6x13", {}, 2, 6, 13, 158 }, { "bold", "7x13B*", {}, 3, 7, 13, 174 } };
    decltype(styles_)::iterator style_{ styles_.begin() };
    int window_w_{ style_->window_width };
    int window_h_{ 1 };

  public:
    gui(config* config, proc* proc)
        : config_(config)
        , proc_(proc)
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
            window_w_,
            window_h_,
            0,
            vinfo.depth,
            InputOutput,
            vinfo.visual,
            CWColormap | CWBorderPixel | CWBackPixel,
            &attr);

        XStoreName(display_, window_, "Gambal");
        auto event_mask = ExposureMask | PointerMotionMask | LeaveWindowMask | EnterWindowMask | ButtonPressMask | ButtonReleaseMask;
        XSelectInput(display_, window_, event_mask);

        auto prop_type = XInternAtom(display_, "_NET_WM_WINDOW_TYPE", False);
        auto prop_data = XInternAtom(display_, "_NET_WM_WINDOW_TYPE_DOCK", False);
        XChangeProperty(display_, window_, prop_type, XA_ATOM, 32, PropModeReplace, reinterpret_cast<unsigned char*>(&prop_data), 1);

        XMapWindow(display_, window_);

        const auto display_w = DisplayWidth(display_, DefaultScreen(display_));
        const auto display_h = DisplayHeight(display_, DefaultScreen(display_));
        const auto window_x = config_->window_x();
        const auto window_y = config_->window_y();
        /* if window position is outside the screen (screen resolution have been changed) */
        if (window_x < 0 || window_x > display_w || window_y < 0 || window_y > display_h)
            XMoveWindow(display_, window_, display_w - 220, display_h - 120);
        else
            XMoveWindow(display_, window_, window_x, window_y);

        buttons_.push_back({ "prev_nic", "<", [&] { proc_->select_prev_nic(); } });
        buttons_.push_back({ "next_nic", ">", [&] { proc_->select_next_nic(); } });
        buttons_.push_back({ "prev_style", "<", [&] { select_prev_style(); } });
        buttons_.push_back({ "next_style", ">", [&] { select_next_style(); } });

        for (auto& style : styles_)
            style.font = XLoadFont(display_, style.font_name.c_str());

        style_ = std::find_if(styles_.begin(), styles_.end(), [&](const auto& style) { return style.name == config_->style_name(); });
        if (style_ == styles_.end())
            style_ = styles_.begin();

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
                    proc_->update();
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
                            if (e.x >= btn.coord.x && e.x < btn.coord.x + btn.dimn.w && e.y >= btn.coord.y && e.y < btn.coord.y + btn.dimn.h)
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
                        window_extended_ = true;
                        send_expose_event = true;
                        break;
                    }
                    case LeaveNotify:
                    {
                        const auto e = xevent.xcrossing;
                        if (!(e.state & Button1Mask)) /* Ensure LeaveNotify is not due to click */
                        {
                            for (auto& btn : buttons_)
                            {
                                if (btn.hovered)
                                    btn.hovered = false;
                            }
                            window_extended_ = false;
                            send_expose_event = true;
                        }
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

        XDestroyWindow(display_, window_);
        XCloseDisplay(display_);

        for (const auto& poll_fd : poll_fds_)
            close(poll_fd.fd);
    }

  private:
    void fill_rectangle(const std::string& gc_name, coord coord, dimn dimn)
    {
        XFillRectangle(display_, window_, gcs_.at(gc_name), coord.x + 01, coord.y + 01, dimn.w, dimn.h);
    }
    void draw_rectangle(const std::string& gc_name, coord coord, dimn dimn)
    {
        XDrawRectangle(display_, window_, gcs_.at(gc_name), coord.x + 01, coord.y + 01, dimn.w - 1, dimn.h - 1);
    }

    void draw_string(const std::string& gc_name, coord coord, const std::string& str)
    {
        XDrawString(display_, window_, gcs_.at(gc_name), coord.x + 01, coord.y + 01, str.data(), str.size());
    }

    void draw_string_center(const std::string& gc_name, coord coord, dimn dimn, std::string str)
    {
        auto extra = static_cast<int>(str.size()) - dimn.w / style_->char_w;
        if (extra > 0)
            str = str.substr(0, str.size() - extra - 2) + "..";

        auto x = coord.x + std::ceil(dimn.w / 2.0 - str.size() * style_->char_w / 2.0);
        auto y = coord.y + std::ceil(dimn.h / 2.0 + style_->char_h / 3.0);
        XDrawString(display_, window_, gcs_.at(gc_name), x + 01, y + 01, str.data(), str.size());
    }

    void draw_line(const std::string& gc_name, coord coord, dimn dimn)
    {
        XDrawLine(display_, window_, gcs_.at(gc_name), coord.x + 01, coord.y + 01, coord.x + 01 + dimn.w, coord.y + 01 - dimn.h);
    }

    void update_gcs()
    {
        struct gc_spec
        {
            std::string name;
            uint8_t r, g, b;
        };

        std::array<gc_spec, 9> gc_specs{ { { "rx", 0x22, 0x8B, 0x22 },
                                           { "tx", 0xDC, 0x14, 0x3C },
                                           { "rxtx", 0xFF, 0xA5, 0x00 },
                                           { "prime", 0x1A, 0x90, 0xFF },
                                           { "purple", 0xBF, 0x90, 0xFF },
                                           { "light", 0xBA, 0xDD, 0xFF },
                                           { "button", 0xF0, 0xF6, 0xFF },
                                           { "button_hover", 0xDA, 0xEC, 0xFF },
                                           { "background", 0xFF, 0xFF, 0xFF } } };

        for (const auto& gc_spec : gc_specs)
        {
            if (gcs_.find(gc_spec.name) == gcs_.end())
                gcs_.emplace(gc_spec.name, XCreateGC(display_, window_, 0, nullptr));

            XSetLineAttributes(display_, gcs_.at(gc_spec.name), 1, LineSolid, CapNotLast, JoinMiter);
            XSetFont(display_, gcs_.at(gc_spec.name), style_->font);
            uint8_t alpha = config_->opacity() / 100.0 * 0xFF;
            uint32_t rgba = alpha << 24 | (gc_spec.r * alpha) / 0xFF << 16 | (gc_spec.g * alpha) / 0xFF << 8 | (gc_spec.b * alpha) / 0xFF;
            XSetForeground(display_, gcs_.at(gc_spec.name), rgba);
        }
    }

    void select_prev_style()
    {
        if (styles_.begin() == style_)
            return;
        style_ = std::prev(style_);
        update_gcs();
        config_->style_name(style_->name);
    }

    void select_next_style()
    {
        if (styles_.end() == std::next(style_))
            return;
        style_ = std::next(style_);
        update_gcs();
        config_->style_name(style_->name);
    }

    auto draw_net(int top)
    {
        const auto org_top = top;
        const auto margin = 5;
        const auto& nic = proc_->selected_nic();
        const auto w_slots = window_w_ - 10 * style_->char_w - 10;
        const auto max_rate = nic.max_rate(w_slots);

        top += style_->char_h + 1;
        draw_string("prime", coord{ margin, top }, humanize_size(max_rate) + "/s");
        top += style_->char_h + 1;
        draw_string("rx", coord{ margin, top }, humanize_size(nic.latest_rate().rx) + "/s");
        top += style_->char_h + 1;
        draw_string("tx", coord{ margin, top }, humanize_size(nic.latest_rate().tx) + "/s");
        top += 4;
        draw_line("light", coord{ margin, top }, dimn{ window_w_ - 2 * margin, 0 });

        auto rate_it = nic.rates().rbegin();
        const auto h_slots = top - org_top - margin;
        for (auto i = 1; i < w_slots && rate_it != nic.rates().rend(); i++, rate_it++)
        {
            if (max_rate == 0)
                break;

            const auto draw_candle = [&](const std::string& gc_name, uint64_t rate) {
                const auto candle = static_cast<int>(rate * h_slots / max_rate);
                draw_line(gc_name, coord{ window_w_ - margin - i, top }, dimn{ 0, candle });
            };

            if (rate_it->rx >= rate_it->tx)
                draw_candle("rx", rate_it->rx);
            else
                draw_candle("tx", rate_it->tx);

            draw_candle("rxtx", std::min(rate_it->rx, rate_it->tx));
        }

        top += style_->char_h + 1;
        draw_string("prime", coord{ margin, top }, "D " + humanize_size(nic.total_bytes().rx, 6));
        draw_string("prime", coord{ window_w_ / 2, top }, "U " + humanize_size(nic.total_bytes().tx, 6));
        top += margin;
        draw_rectangle("light", coord{ 0, org_top }, dimn{ window_w_, top - org_top });
        return top;
    }

    auto draw_cpu(int top)
    {
        const auto& cpus = proc_->cpus();
        const auto cpu_count = cpus.size();
        const auto thickness = style_->thickness;
        const auto gap = 2;

        const auto rows = std::ceil(cpu_count / 8.0);
        const auto cols = std::ceil(cpu_count / rows);
        const auto f_width = (window_w_ - (cols - 1) * gap) / cols;

        auto cpu_it = cpus.begin();
        for (auto r = 0; r < rows; r++)
        {
            for (auto c = 0; c < cols && cpu_it != cpus.end(); c++, cpu_it++)
            {
                const auto x1 = static_cast<int>(std::floor(f_width * c + c * gap));
                const auto x2 = static_cast<int>(std::floor(f_width * c + c * gap + f_width));
                const auto width = x2 - x1;
                const auto candle = static_cast<int>(cpu_it->usage() * width);
                fill_rectangle("light", coord{ x1, top }, dimn{ width, thickness });
                fill_rectangle("prime", coord{ x1, top }, dimn{ candle, thickness });
            }
            top += gap + thickness;
        }
        return top;
    }

    auto draw_ram(int top)
    {
        const auto thickness = style_->thickness;

        const auto& mem = proc_->get_mem();
        const auto used = static_cast<int>(1.0 * mem.used / mem.total * window_w_);
        const auto buffcache = static_cast<int>(1.0 * mem.buffcache / mem.total * window_w_);

        fill_rectangle("light", coord{ 0, top }, dimn{ window_w_, thickness });
        fill_rectangle("prime", coord{ 0, top }, dimn{ used, thickness });
        fill_rectangle("purple", coord{ 0, top } + coord{ used, 0 }, dimn{ buffcache, thickness });
        return top + thickness;
    }

    auto draw_opt(int top)
    {
        if (!window_extended_)
            return top;
        top += 2;
        const auto org_top = top;
        const auto bsize = 17;
        top += 1;
        for (auto& btn : buttons_)
        {
            btn.dimn = { bsize, bsize };
            if (btn.name == "prev_nic")
            {
                btn.disabled = !proc_->is_prev_nic_available();
                btn.coord = coord{ 1, top };
            }

            if (btn.name == "next_nic")
            {
                btn.disabled = !proc_->is_next_nic_available();
                btn.coord = coord{ window_w_ - bsize - 1, top };
            }

            if (btn.name == "prev_style")
            {
                btn.disabled = style_ == styles_.begin();
                btn.coord = coord{ 1, top + bsize + 1 };
            }

            if (btn.name == "next_style")
            {
                btn.disabled = style_ == std::prev(styles_.end());
                btn.coord = coord{ window_w_ - bsize - 1, top + bsize + 1 };
            }

            if (btn.hovered && !btn.disabled)
                fill_rectangle("button_hover", btn.coord, btn.dimn);
            else
                fill_rectangle("button", btn.coord, btn.dimn);

            if (!btn.disabled)
                draw_string_center("prime", btn.coord, btn.dimn, btn.text);
        }

        draw_string_center("prime", coord{ bsize + 1, top }, dimn{ window_w_ - 2 * (bsize + 1), bsize }, proc_->selected_nic_name());
        top += bsize;
        draw_line("light", coord{ 0, top }, dimn{ window_w_, 0 });
        top += 1;
        draw_string_center("prime", coord{ bsize + 1, top }, dimn{ window_w_ - 2 * (bsize + 1), bsize }, style_->name);
        top += bsize + 1;
        draw_rectangle("light", coord{ 0, org_top }, dimn{ window_w_, top - org_top });

        return top;
    }

    void expose()
    {
        fill_rectangle("background", coord{ -01, -01 }, dimn{ window_w_ + 02, window_h_ + 01 });

        auto top = 0;
        top = draw_net(top) + 2;
        top = draw_cpu(top);
        top = draw_ram(top);
        top = draw_opt(top) + 1;

        if (top != window_h_ || window_w_ != style_->window_width)
        {
            window_w_ = style_->window_width;
            window_h_ = top;
            XResizeWindow(display_, window_, window_w_ + 02, window_h_ + 01);
        }
    }

    static std::string humanize_size(uint64_t size, size_t max_width = 4)
    {
        std::array<const char*, 7> units = { "B", "KB", "MB", "GB", "TB", "PB", "EB" };

        const auto str = std::to_string(size);

        if (str.size() <= 3)
            return { str + " " + units[0] };

        const auto order = static_cast<size_t>(std::ceil((str.size() - 3) / 3.0));

        auto int_digits = str.size() % 3;
        if (int_digits == 0)
            int_digits = 3;

        const auto fraction_digits = max_width - int_digits - 1;
        if (!fraction_digits)
            return str.substr(0, 3) + " " + units[order];

        return { str.substr(0, int_digits) + "." + str.substr(int_digits, std::min(fraction_digits, order)) + " " + units[order] };
    }
};
} // namespace gambal