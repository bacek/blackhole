#include "blackhole/record.hpp"

#include <thread>

#include <iostream>

namespace blackhole {

struct record_t::inner_t {
    int severity;
    time_point timestamp;
    std::reference_wrapper<const string_view> message;
    std::reference_wrapper<const string_view> formatted;

    pid_t pid;
    std::thread::id tid;

    const attribute_pack* attributes;

    static auto cast(storage_type& storage) noexcept -> inner_t& {
        return reinterpret_cast<inner_t&>(storage);
    }

    static auto cast(const storage_type& storage) noexcept -> const inner_t& {
        return reinterpret_cast<const inner_t&>(storage);
    }
};

record_t::record_t(int severity, const string_view& message, const attribute_pack& attributes) {
    static_assert(sizeof(inner_t) <= sizeof(record_t), "padding or alignment violation");

    auto& inner = inner_t::cast(storage);
    inner.message = message;
    inner.severity = severity;
}

auto record_t::message() const noexcept -> const string_view& {
    return inner_t::cast(storage).message.get();
}

auto record_t::severity() const noexcept -> int {
    return inner_t::cast(storage).severity;
}

}  // namespace blackhole