#pragma once
#include <initializer_list>
#include <utility>
#include <vector>

template <typename Key, typename Value, typename Compare = std::less<Key>>
class Map {
public:
    Map() = default;

    Value& operator[](const Key& key) {
        for (auto& kv : data_) {
            if (!comp_(kv.first, key) && !comp_(key, kv.first)) {
                return kv.second;
            }
        }
        size_t pos = 0;
        while (pos < data_.size() && comp_(data_[pos].first, key)) {
            ++pos;
        }
        data_.insert(data_.begin() + pos, std::make_pair(key, Value{}));
        return data_[pos].second;
    }

    inline bool IsEmpty() const noexcept {
        return data_.empty();
    }

    inline size_t Size() const noexcept {
        return data_.size();
    }

    void Swap(Map& a) {
        using std::swap;
        swap(data_, a.data_);
    }

    std::vector<std::pair<const Key, Value>> Values(bool is_increase = true) const noexcept {
        std::vector<std::pair<const Key, Value>> res;
        res.reserve(data_.size());
        if (is_increase) {
            for (const auto& kv : data_) {
                res.emplace_back(kv.first, kv.second);
            }
        } else {
            for (size_t i = data_.size(); i > 0; --i) {
                const auto& kv = data_[i - 1];
                res.emplace_back(kv.first, kv.second);
            }
        }
        return res;
    }

    void Insert(const std::pair<const Key, Value>& val) {
        const Key& key = val.first;
        for (auto& kv : data_) {
            if (!comp_(kv.first, key) && !comp_(key, kv.first)) {
                kv.second = val.second;
                return;
            }
        }
        size_t pos = 0;
        while (pos < data_.size() && comp_(data_[pos].first, key)) {
            ++pos;
        }
        data_.insert(data_.begin() + pos, std::make_pair(key, val.second));
    }

    void Insert(const std::initializer_list<std::pair<const Key, Value>>& values) {
        for (const auto& v : values) {
            Insert(v);
        }
    }

    void Erase(const Key& key) {
        for (size_t i = 0; i < data_.size(); ++i) {
            if (!comp_(data_[i].first, key) && !comp_(key, data_[i].first)) {
                data_.erase(data_.begin() + i);
                return;
            }
        }
    }

    void Clear() noexcept {
        data_.clear();
    }

    bool Find(const Key& key) const {
        for (const auto& kv : data_) {
            if (!comp_(kv.first, key) && !comp_(key, kv.first)) {
                return true;
            }
        }
        return false;
    }

    ~Map() = default;

private:
    Compare comp_{};
    std::vector<std::pair<Key, Value>> data_{};
};
