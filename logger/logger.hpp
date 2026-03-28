#pragma once
#include <format>
#include <memory>
#include <mutex>

#include "color.hpp"
#include "level.hpp"
#include "printer.hpp"

namespace clear_server::logger {

class DefaultLogger {
public:
    DefaultLogger(Level level = Level::Info)
        : level_{level}
        , mutex_{std::make_unique<std::mutex>()} {} 

    template <typename... Args>
    void info(std::format_string<Args...> fmt, Args... args) {
        log(InfoPrinter(), fmt, std::forward<Args>(args)...);
    }

    template <typename... Args>
    void error(std::format_string<Args...> fmt, Args... args) {
        log(ErrorPrinter(), fmt, std::forward<Args>(args)...);
    }

    template <typename... Args>
    void debug(std::format_string<Args...> fmt, Args... args) {
        log(DebugPrinter(), fmt, std::forward<Args>(args)...);
    }

private:
    template <color::ColorType Color, typename... Args>
    void log(const PrinterBase<Color>& printer, std::format_string<Args...> fmt, Args... args) {
        if (printer.available(level_)) {
            std::lock_guard lock{*mutex_};
            printer.print(fmt, std::forward<Args>(args)...);
        }
    }

    Level level_;
    std::unique_ptr<std::mutex> mutex_;
};

} // namespace clear_server::logger
