#pragma once

#include <fstream>
#include <string>

namespace gambal
{
class config
{
    std::string path_;
    std::string style_name_{ "-" };
    std::string nic_name_{ "-" };
    unsigned int opacity_{ 90 };
    int window_x_{ -1 };
    int window_y_{ -1 };

  public:
    explicit config(std::string path)
        : path_(std::move(path))
    {
        std::ifstream ifs{ path_ };
        if (ifs.is_open())
            ifs >> style_name_ >> nic_name_ >> opacity_ >> window_x_ >> window_y_;
    }

    const auto& style_name() const
    {
        return style_name_;
    }

    auto style_name(const std::string& style_name)
    {
        style_name_ = style_name;
        update_settings_file();
    }

    const auto& nic_name() const
    {
        return nic_name_;
    }

    auto nic_name(const std::string& nic_name)
    {
        nic_name_ = nic_name;
        update_settings_file();
    }

    auto opacity() const
    {
        return opacity_;
    }

    auto opacity(unsigned int opacity)
    {
        opacity_ = opacity;
        update_settings_file();
    }

    auto window_xy() const
    {
        return std::pair{ window_x_, window_y_ };
    }

    auto window_xy(int x, int y)
    {
        window_x_ = x;
        window_y_ = y;
        update_settings_file();
    }

  private:
    void update_settings_file()
    {
        std::ofstream ofs{ path_ };
        if (ofs.is_open())
            ofs << style_name_ << '\n' << nic_name_ << '\n' << opacity_ << '\n' << window_x_ << '\n' << window_y_ << '\n';
    }
};
} // namespace gambal