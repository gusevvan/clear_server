#pragma once
#include <format>

namespace clear_server::logger {

template <typename T, typename... Args>
concept LoggerType = requires(T t, std::format_string<Args...> fmt, Args&&... args) {
    t.info(fmt, std::forward<Args>(args)...);
    t.error(fmt, std::forward<Args>(args)...);
    t.debug(fmt, std::forward<Args>(args)...);
};

} // namespace clear_server::logger
