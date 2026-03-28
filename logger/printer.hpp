#pragma once
#include <chrono>
#include <concepts>
#include <format>
#include <iostream>
#include <string>

#include "color.hpp"
#include "level.hpp"

namespace clear_server::logger {

template <color::ColorType Color>
class PrinterBase {
protected:
    PrinterBase(std::string type, Level level) 
        : type_{std::move(type)}, level_{level} {}

    friend class DefaultLogger;
    
    template <typename... Args>
    void print(std::format_string<Args...> fmt, Args... args) const {
        std::cout << std::format("[{}]", getTimeFmt()) << " [" << color_ << 
            std::format("{}", type_) << color::Default() << "] " <<
            std::format(fmt, std::forward<Args>(args)...) << "\n";
    }

    bool available(Level level) const {
        return level >= level_;
    }

private:
    std::string getTimeFmt() const {
        auto now = std::chrono::system_clock::now();
        auto local = std::chrono::zoned_time{std::chrono::current_zone(), now};
        auto local_ms = std::chrono::time_point_cast<std::chrono::milliseconds>(
            local.get_local_time()
        );
        return std::format("{:%Y-%m-%d %H:%M:%S}", local_ms);
    }
    
    Level level_;
    Color color_;
    std::string type_;
};

class DebugPrinter final : public PrinterBase<color::Yellow> {
public:
    DebugPrinter() : PrinterBase("debug", Level::Debug) {}
};

class InfoPrinter final : public PrinterBase<color::Green> {
public:
    InfoPrinter() : PrinterBase("info", Level::Info) {}
};

class ErrorPrinter final : public PrinterBase<color::Red> {
public:
    ErrorPrinter() : PrinterBase("error", Level::Error) {}
};

} // namespace clear_server::logger
