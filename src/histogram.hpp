#pragma once

#include <array>
#include <iterator>

namespace gambal
{
template<typename T, size_t N>
class histogram
{
    static constexpr const auto S = N + 1;
    std::array<T, S> buffer_{};
    size_t pos_{};

  public:
    auto push_back(T value)
    {
        pos_ = (pos_ + 1) % S;
        buffer_[pos_] = value;
    }

    auto static constexpr size()
    {
        return N;
    }

    class iterator
    {
        const T* buffer_;
        size_t pos_;

      public:
        using iterator_category = std::bidirectional_iterator_tag;
        using value_type = const T;
        using difference_type = std::ptrdiff_t;
        using pointer = value_type*;
        using reference = value_type&;

        iterator(const T* buffer, size_t pos)
            : buffer_(buffer)
            , pos_(pos)
        {
        }

        auto const& operator*() const
        {
            return *(buffer_ + pos_);
        }

        auto operator->() const
        {
            return buffer_ + pos_;
        }

        auto& operator++()
        {
            pos_ = (pos_ + S - 1) % S;
            return *this;
        }

        auto& operator--()
        {
            pos_ = (pos_ + 1) % S;
            return *this;
        }

        auto operator++(int)
        {
            auto tmp = *this;
            ++*this;
            return tmp;
        }

        auto operator--(int)
        {
            auto tmp = *this;
            --*this;
            return tmp;
        }

        auto operator==(const iterator& lhs) const
        {
            return lhs.pos_ == pos_ && lhs.buffer_ == buffer_;
        }

        auto operator!=(const iterator& lhs) const
        {
            return !(operator==(lhs));
        }
    };
    using reverse_iterator = std::reverse_iterator<iterator>;

    auto begin() const
    {
        return iterator(buffer_.data(), pos_);
    }

    auto end() const
    {
        return iterator(buffer_.data(), (pos_ + 1) % S);
    }
};
} // namespace gambal
