#pragma once
#include <ostream>
#include <format>

namespace clear_server::logger {

namespace color {

class Color {
protected:
    Color(uint16_t color_code) : color_code_{color_code} {}
    friend std::ostream& operator<<(std::ostream& os, const Color& color);

private:
    uint16_t color_code_;
};

std::ostream& operator<<(std::ostream& stream, const Color& color) {
    stream << std::format("\033[{}m", color.color_code_);
    return stream;
}

template <typename T>
concept ColorType = std::derived_from<T, Color>;

class Default final : public Color {
public:
    Default() : Color(0) {}
};

class Red final : public Color {
public:
    Red() : Color(31) {}
};

class Green final : public Color {
public:
    Green() : Color(32) {}
};

class Yellow final : public Color {
public:
    Yellow() : Color(33) {}
};

class Blue final : public Color {
public:
    Blue() : Color(34) {}
};

} // namespace color

} // namespace clear_server::logger
