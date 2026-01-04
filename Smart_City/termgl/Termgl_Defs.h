#pragma once
#ifndef TERMGL_DEFS_H
#define TERMGL_DEFS_H

#include <cstdint>
#include <cmath>

namespace termgl {

    struct Color {
        uint8_t b, g, r, a;

        Color(int _r, int _g, int _b, int _a = 255) : r(static_cast<uint8_t>(_r)), g(static_cast<uint8_t>(_g)), b(static_cast<uint8_t>(_b)), a(static_cast<uint8_t>(_a)) {}
        Color() : r(0), g(0), b(0), a(255) {}

        // Predefined Colors
        static Color Black() { return Color(0, 0, 0); }
        static Color White() { return Color(255, 255, 255); }
        static Color Red() { return Color(255, 50, 50); }
        static Color Green() { return Color(50, 255, 50); }
        static Color Blue() { return Color(50, 100, 255); }
        static Color Grey() { return Color(128, 128, 128); }
        static Color Yellow() { return Color(255, 255, 0); }
		static Color Cyan() { return Color(0, 255, 255); }

        uint32_t toInt() const {
            return (static_cast<uint32_t>(a) << 24) | (static_cast<uint32_t>(r) << 16) | (static_cast<uint32_t>(g) << 8) | static_cast<uint32_t>(b);
        }
    };

    struct Vec2 {
        int x, y;
        Vec2() : x(0), y(0) {}
        Vec2(int _x, int _y) : x(_x), y(_y) {}

        float distance(const Vec2& other) const {
            return std::sqrt(static_cast<float>(std::pow(x - other.x, 2) + std::pow(y - other.y, 2)));
        }
    };

    struct Rect {
        int x, y, w, h;
        Rect(int _x, int _y, int _w, int _h) : x(_x), y(_y), w(_w), h(_h) {}
        Rect() : x(0), y(0), w(0), h(0) {}
    };

} // namespace termgl

#endif // TERMGL_DEFS_H