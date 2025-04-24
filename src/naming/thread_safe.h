#pragma once

template <typename T>
class ThreadSafe {
public:
    ThreadSafe() = default;
    explicit ThreadSafe(const T& value) : data_(value) {} // safe copy for existing objects
    explicit ThreadSafe(T&& value) : data_(std::move(value)) {} // efficient move

    template <typename Func>
    auto read(Func&& fn) const {
        std::shared_lock lock(mu_);
        return fn(data_);
    }

    template <typename Func>
    auto write(Func&& fn) {
        std::unique_lock lock(mu_);
        return fn(data_);
    }

private:
    mutable std::shared_mutex mu_;
    T data_;
};
