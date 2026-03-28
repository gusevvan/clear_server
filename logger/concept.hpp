#pragma once
#include <format>

namespace clear_server::logger {

template <typename T, typename... Args>
concept LoggerType = requires(T t, std::format_string<Args...> fmt, Args... args) {
    t.info(fmt, args...);
    t.error(fmt, args...);
    t.debug(fmt, args...);
};

} // namespace clear_server::logger
