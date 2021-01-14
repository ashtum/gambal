#pragma once

#include "ring_buffer.hpp"

#include <algorithm>
#include <fstream>
#include <limits>
#include <map>
#include <sstream>

namespace ashmon
{
class nic
{
  public:
    struct rxtx
    {
        uint64_t rx{};
        uint64_t tx{};

        rxtx& operator+=(const rxtx& rhs)
        {
            rx += rhs.rx;
            tx += rhs.tx;
            return *this;
        }
    };

  private:
    rxtx latest_bytes_{ std::numeric_limits<uint64_t>::max(), std::numeric_limits<uint64_t>::max() };
    rxtx total_bytes_{};
    ring_buffer<rxtx, 100> rates_;

  public:
    void update(const rxtx& current_bytes)
    {
        rxtx rate{};

        /* check for rollover */
        if (latest_bytes_.rx <= current_bytes.rx)
            rate.rx = current_bytes.rx - latest_bytes_.rx;

        /* check for rollover */
        if (latest_bytes_.tx <= current_bytes.tx)
            rate.tx = current_bytes.tx - latest_bytes_.tx;

        rates_.push_back(rate);
        total_bytes_ += rate;
        latest_bytes_ = current_bytes;
    }

    const auto& rates() const
    {
        return rates_;
    }

    const auto& total_bytes() const
    {
        return total_bytes_;
    }

    const auto& latest_rate() const
    {
        return *rates_.rbegin();
    }

    auto max_rate() const
    {
        auto max_rate = *std::max_element(rates_.begin(), rates_.end(), [](const auto& lhs, const auto& rhs) {
            return std::max(lhs.rx, lhs.tx) < std::max(rhs.rx, rhs.tx);
        });
        return std::max(max_rate.rx, max_rate.tx);
    }
};

class nic_mgr
{
    config* config_;
    std::ifstream ifs_;
    std::map<std::string, nic> nics_;
    decltype(nics_)::iterator selected_nic_{};

  public:
    explicit nic_mgr(config* config)
        : config_(config)
        , ifs_("/proc/net/dev")
    {
        if (!ifs_.is_open())
            throw std::runtime_error("unable to open file at /proc/net/dev");

        update();

        if (nics_.empty())
            throw std::runtime_error("no network interface available");

        selected_nic_ = nics_.find(config_->nic_name());
        if (selected_nic_ == nics_.end())
            selected_nic_ = nics_.begin();
    }

    void update()
    {
        std::string line;

        /* ignore header lines */
        std::getline(ifs_, line);
        std::getline(ifs_, line);

        while (std::getline(ifs_, line))
        {
            std::istringstream iss(line);

            std::string nic_name;
            nic::rxtx current_bytes{};

            iss >> nic_name;
            nic_name.pop_back(); /* remove the ':' character from the end */

            iss >> current_bytes.rx;

            std::string ignore;
            for (auto i = 0; i < 7; i++)
                iss >> ignore;

            iss >> current_bytes.tx;

            auto& nic = nics_[nic_name];
            nic.update(current_bytes);
        }
        ifs_.clear();
        ifs_.seekg(0);
    }

    auto is_next_nic_available() const
    {
        return std::next(selected_nic_) != nics_.end();
    }

    auto is_prev_nic_available() const
    {
        return selected_nic_ != nics_.begin();
    }

    const auto& selected_nic() const
    {
        return selected_nic_->second;
    }

    const auto& selected_nic_name() const
    {
        return selected_nic_->first;
    }

    void select_prev_nic()
    {
        if (!is_prev_nic_available())
            throw std::runtime_error{ "there is no prev nic" };
        selected_nic_ = std::prev(selected_nic_);
        config_->nic_name(selected_nic_name());
    }

    void select_next_nic()
    {
        if (!is_next_nic_available())
            throw std::runtime_error{ "there is no next nic" };
        selected_nic_ = std::next(selected_nic_);
        config_->nic_name(selected_nic_name());
    }
};
} // namespace ashmon