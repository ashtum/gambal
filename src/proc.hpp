#pragma once

#include "config.hpp"
#include "ring_buffer.hpp"

#include <algorithm>
#include <fstream>
#include <iostream>
#include <limits>
#include <map>
#include <sstream>
#include <vector>

namespace gambal
{
inline auto wrap_subtract(uint64_t lhs, uint64_t rhs)
{
    if (lhs > rhs)
        return lhs - rhs;
    return uint64_t{};
}

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
    ring_buffer<rxtx, 200> rates_;

  public:
    void update(const rxtx& current_bytes)
    {
        rxtx rate{};

        rate.rx = wrap_subtract(current_bytes.rx, latest_bytes_.rx);
        rate.tx = wrap_subtract(current_bytes.tx, latest_bytes_.tx);

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

    auto max_rate(size_t n) const
    {
        const auto end = std::next(rates_.rbegin(), n);
        const auto max_rate =
            *std::max_element(rates_.rbegin(), end, [](const auto& lhs, const auto& rhs) { return std::max(lhs.rx, lhs.tx) < std::max(rhs.rx, rhs.tx); });
        return std::max(max_rate.rx, max_rate.tx);
    }
};

class cpu
{
    float usage_{};
    uint64_t last_totaltime_{ std::numeric_limits<uint64_t>::max() };
    uint64_t last_loadtime_{ std::numeric_limits<uint64_t>::max() };

  public:
    void update(uint64_t totaltime, uint64_t loadtime)
    {
        auto total_period = wrap_subtract(totaltime, last_totaltime_);
        auto load_period = wrap_subtract(loadtime, last_loadtime_);

        if (total_period)
            usage_ = load_period * 1.0 / total_period;

        last_totaltime_ = totaltime;
        last_loadtime_ = loadtime;
    }

    auto usage() const
    {
        return usage_;
    }
};

struct mem
{
    uint64_t total{};
    uint64_t used{};
    uint64_t buffcache{};
};

class proc
{
    config* config_;
    std::ifstream net_ifs_;
    std::ifstream stat_ifs_;
    std::ifstream meminfo_ifs_;
    std::map<std::string, nic> nics_;
    std::vector<cpu> cpus_;
    mem mem_;
    decltype(nics_)::iterator selected_nic_{};

  public:
    explicit proc(config* config)
        : config_(config)
        , net_ifs_("/proc/net/dev")
        , stat_ifs_("/proc/stat")
        , meminfo_ifs_("/proc/meminfo")
    {
        if (!net_ifs_.is_open())
            throw std::runtime_error("unable to open file at /proc/net/dev");

        if (!stat_ifs_.is_open())
            throw std::runtime_error("unable to open file at /proc/stat");

        if (!meminfo_ifs_.is_open())
            throw std::runtime_error("unable to open file at /proc/meminfo");

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

        net_ifs_.clear();
        net_ifs_.seekg(0);
        std::getline(net_ifs_, line); /* ignore header lines */
        std::getline(net_ifs_, line); /* ignore header lines */

        while (std::getline(net_ifs_, line))
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

        stat_ifs_.clear();
        stat_ifs_.seekg(0);
        std::getline(stat_ifs_, line); /* ignore header lines */

        while (std::getline(stat_ifs_, line))
        {
            std::istringstream iss(line);
            std::string tag;
            iss >> tag;
            if (tag.substr(0, 3) != "cpu")
                break;

            const auto index = std::stoul(tag.substr(3));

            uint64_t user = 0, nice = 0, system = 0, idle = 0, iowait = 0, irq = 0, softirq = 0, steal = 0;
            iss >> user >> nice >> system >> idle >> iowait >> irq >> softirq >> steal;
            const auto usagetime = user + nice + system + irq + softirq + steal;
            const auto totaltime = idle + iowait + usagetime;

            if (cpus_.size() <= index)
                cpus_.resize(index + 1);
            cpus_.at(index).update(totaltime, usagetime);
        }

        meminfo_ifs_.clear();
        meminfo_ifs_.seekg(0);
        uint64_t total = 0, free = 0, buff = 0, cache = 0, reclaimable = 0;
        while (std::getline(meminfo_ifs_, line))
        {
            std::istringstream iss(line);
            std::string tag;
            iss >> tag;

            if (tag == "MemTotal:")
                iss >> total;
            else if (tag == "MemFree:")
                iss >> free;
            else if (tag == "Buffers:")
                iss >> buff;
            else if (tag == "Cached:")
                iss >> cache;
            else if (tag == "SReclaimable:")
                iss >> reclaimable;
        }
        mem_.total = total;
        mem_.buffcache = buff + cache + reclaimable;
        mem_.used = total - free - mem_.buffcache;
    }

    const auto& get_mem() const
    {
        return mem_;
    }

    const auto& cpus() const
    {
        return cpus_;
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
} // namespace gambal