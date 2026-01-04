// Prevent Windows min/max macros from conflicting with std::min/std::max
#ifndef NOMINMAX
#define NOMINMAX
#endif

#include "Termgl.h"
#include <cmath>
#include <algorithm>
#include <stdexcept>
#include <chrono>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

namespace termgl {

    // ============================================================================
    // INTERNAL FONT DATA (8x16)
    // ============================================================================
    static const unsigned char FONT8x16[128][16] = {
        {0}, {0}, {0}, {0}, {0}, {0}, {0}, {0}, {0}, {0}, {0}, {0}, {0}, {0}, {0}, {0},
        {0}, {0}, {0}, {0}, {0}, {0}, {0}, {0}, {0}, {0}, {0}, {0}, {0}, {0}, {0}, {0},
        // 32: SPACE
        {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},
        // 33: !
        {0x00,0x00,0x18,0x3C,0x3C,0x3C,0x18,0x18,0x18,0x00,0x18,0x18,0x00,0x00,0x00,0x00},
        // 34: "
        {0x00,0x66,0x66,0x66,0x24,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},
        // 35: #
        {0x00,0x00,0x6C,0x6C,0xFE,0x6C,0x6C,0x6C,0xFE,0x6C,0x6C,0x00,0x00,0x00,0x00,0x00},
        // 36: $
        {0x18,0x18,0x7C,0xC6,0xC2,0xC0,0x7C,0x06,0x06,0x86,0xC6,0x7C,0x18,0x18,0x00,0x00},
        // 37: %
        {0x00,0x00,0x00,0xC6,0xCC,0x18,0x30,0x60,0xC0,0x66,0xC6,0x00,0x00,0x00,0x00,0x00},
        // 38: &
        {0x00,0x00,0x38,0x6C,0x6C,0x38,0x76,0xDC,0xCC,0xCC,0x76,0x00,0x00,0x00,0x00,0x00},
        // 39: '
        {0x00,0x18,0x18,0x18,0x08,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},
        // 40: (
        {0x00,0x00,0x0C,0x18,0x30,0x30,0x30,0x30,0x30,0x30,0x18,0x0C,0x00,0x00,0x00,0x00},
        // 41: )
        {0x00,0x00,0x30,0x18,0x0C,0x0C,0x0C,0x0C,0x0C,0x0C,0x18,0x30,0x00,0x00,0x00,0x00},
        // 42: *
        {0x00,0x00,0x00,0x00,0x18,0x18,0x7E,0x3C,0x7E,0x18,0x18,0x00,0x00,0x00,0x00,0x00},
        // 43: +
        {0x00,0x00,0x00,0x00,0x18,0x18,0x18,0x7E,0x18,0x18,0x18,0x00,0x00,0x00,0x00,0x00},
        // 44: ,
        {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x18,0x18,0x18,0x08,0x00,0x00,0x00},
        // 45: -
        {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x7E,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},
        // 46: .
        {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x18,0x18,0x00,0x00,0x00,0x00,0x00},
        // 47: /
        {0x00,0x00,0x00,0x00,0x02,0x06,0x0C,0x18,0x30,0x60,0xC0,0x80,0x00,0x00,0x00,0x00},
        // 48: 0
        {0x00,0x00,0x3C,0x66,0xC3,0xC3,0xC3,0xC3,0xC3,0xC3,0x66,0x3C,0x00,0x00,0x00,0x00},
        // 49: 1
        {0x00,0x00,0x18,0x38,0x78,0x18,0x18,0x18,0x18,0x18,0x18,0x7E,0x00,0x00,0x00,0x00},
        // 50: 2
        {0x00,0x00,0x3C,0x66,0xC3,0x03,0x06,0x0C,0x18,0x30,0x60,0xFF,0x00,0x00,0x00,0x00},
        // 51: 3
        {0x00,0x00,0x3C,0x66,0xC3,0x03,0x1E,0x03,0xC3,0xC3,0x66,0x3C,0x00,0x00,0x00,0x00},
        // 52: 4
        {0x00,0x00,0x0C,0x1C,0x3C,0x6C,0xCC,0xFE,0x0C,0x0C,0x0C,0x1E,0x00,0x00,0x00,0x00},
        // 53: 5
        {0x00,0x00,0xFF,0xC0,0xC0,0xFC,0x06,0x03,0x03,0xC3,0x66,0x3C,0x00,0x00,0x00,0x00},
        // 54: 6
        {0x00,0x00,0x3C,0x66,0xC0,0xC0,0xFC,0xC6,0xC3,0xC3,0x66,0x3C,0x00,0x00,0x00,0x00},
        // 55: 7
        {0x00,0x00,0xFF,0xC3,0x06,0x0C,0x18,0x30,0x30,0x30,0x30,0x30,0x00,0x00,0x00,0x00},
        // 56: 8
        {0x00,0x00,0x3C,0x66,0xC3,0xC3,0x7E,0xC3,0xC3,0xC3,0x66,0x3C,0x00,0x00,0x00,0x00},
        // 57: 9
        {0x00,0x00,0x3C,0x66,0xC3,0xC3,0x7E,0x03,0x03,0xC3,0x66,0x3C,0x00,0x00,0x00,0x00},
        // 58: :
        {0x00,0x00,0x00,0x00,0x18,0x18,0x00,0x00,0x00,0x18,0x18,0x00,0x00,0x00,0x00,0x00},
        // 59: ;
        {0x00,0x00,0x00,0x00,0x18,0x18,0x00,0x00,0x00,0x18,0x18,0x08,0x00,0x00,0x00,0x00},
        // 60: <
        {0x00,0x00,0x06,0x18,0x60,0xC0,0xC0,0x60,0x18,0x06,0x00,0x00,0x00,0x00,0x00,0x00},
        // 61: =
        {0x00,0x00,0x00,0x00,0x7E,0x00,0x00,0x7E,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},
        // 62: >
        {0x00,0x00,0x60,0x18,0x06,0x03,0x03,0x06,0x18,0x60,0x00,0x00,0x00,0x00,0x00,0x00},
        // 63: ?
        {0x00,0x00,0x3C,0x66,0xC3,0x06,0x0C,0x18,0x18,0x00,0x18,0x18,0x00,0x00,0x00,0x00},
        // 64: @
        {0x00,0x00,0x3C,0x66,0xC3,0xC3,0xDB,0xDB,0xDF,0xD8,0xC0,0xC0,0x7E,0x00,0x00,0x00},
        // 65: A
        {0x00,0x00,0x18,0x3C,0x66,0x66,0xC3,0xC3,0xFF,0xC3,0xC3,0xC3,0x00,0x00,0x00,0x00},
        // 66: B
        {0x00,0x00,0xF8,0x6C,0x66,0x66,0x7C,0x66,0x66,0x66,0x6C,0xF8,0x00,0x00,0x00,0x00},
        // 67: C
        {0x00,0x00,0x3C,0x66,0xC3,0xC0,0xC0,0xC0,0xC0,0xC3,0x66,0x3C,0x00,0x00,0x00,0x00},
        // 68: D
        {0x00,0x00,0xF8,0x6C,0x66,0x66,0x66,0x66,0x66,0x66,0x6C,0xF8,0x00,0x00,0x00,0x00},
        // 69: E
        {0x00,0x00,0xFE,0x62,0x60,0x64,0x7C,0x64,0x60,0x62,0xFE,0x00,0x00,0x00,0x00,0x00},
        // 70: F
        {0x00,0x00,0xFE,0x62,0x60,0x64,0x7C,0x64,0x60,0x60,0xF0,0x00,0x00,0x00,0x00,0x00},
        // 71: G
        {0x00,0x00,0x3C,0x66,0xC3,0xC0,0xC0,0xCF,0xC3,0xC3,0x66,0x3C,0x00,0x00,0x00,0x00},
        // 72: H
        {0x00,0x00,0xC3,0xC3,0xC3,0xC3,0xFF,0xC3,0xC3,0xC3,0xC3,0xC3,0x00,0x00,0x00,0x00},
        // 73: I
        {0x00,0x00,0x3C,0x18,0x18,0x18,0x18,0x18,0x18,0x18,0x18,0x3C,0x00,0x00,0x00,0x00},
        // 74: J
        {0x00,0x00,0x0F,0x06,0x06,0x06,0x06,0x06,0x06,0xC6,0xC6,0x7C,0x00,0x00,0x00,0x00},
        // 75: K
        {0x00,0x00,0xC6,0xCC,0xD8,0xF0,0xE0,0xF0,0xD8,0xCC,0xC6,0xC3,0x00,0x00,0x00,0x00},
        // 76: L
        {0x00,0x00,0xF0,0x60,0x60,0x60,0x60,0x60,0x60,0x62,0xFE,0x00,0x00,0x00,0x00,0x00},
        // 77: M
        {0x00,0x00,0xC3,0xC3,0xE7,0xFF,0xDB,0xC3,0xC3,0xC3,0xC3,0xC3,0x00,0x00,0x00,0x00},
        // 78: N
        {0x00,0x00,0xC3,0xC3,0xE3,0xF3,0xDB,0xCF,0xC7,0xC3,0xC3,0xC3,0x00,0x00,0x00,0x00},
        // 79: O
        {0x00,0x00,0x3C,0x66,0xC3,0xC3,0xC3,0xC3,0xC3,0xC3,0x66,0x3C,0x00,0x00,0x00,0x00},
        // 80: P
        {0x00,0x00,0xFC,0x66,0x66,0x66,0x7C,0x60,0x60,0x60,0xF0,0x00,0x00,0x00,0x00,0x00},
        // 81: Q
        {0x00,0x00,0x3C,0x66,0xC3,0xC3,0xC3,0xC3,0xDB,0xCF,0x66,0x3C,0x0F,0x00,0x00,0x00},
        // 82: R
        {0x00,0x00,0xFC,0x66,0x66,0x66,0x7C,0x6C,0x66,0x66,0x66,0xE3,0x00,0x00,0x00,0x00},
        // 83: S
        {0x00,0x00,0x3C,0x66,0xC3,0x60,0x3C,0x06,0x03,0xC3,0x66,0x3C,0x00,0x00,0x00,0x00},
        // 84: T
        {0x00,0x00,0xFF,0xDB,0x99,0x18,0x18,0x18,0x18,0x18,0x18,0x3C,0x00,0x00,0x00,0x00},
        // 85: U
        {0x00,0x00,0xC3,0xC3,0xC3,0xC3,0xC3,0xC3,0xC3,0xC3,0x66,0x3C,0x00,0x00,0x00,0x00},
        // 86: V
        {0x00,0x00,0xC3,0xC3,0xC3,0xC3,0xC3,0xC3,0x66,0x3C,0x18,0x18,0x00,0x00,0x00,0x00},
        // 87: W
        {0x00,0x00,0xC3,0xC3,0xC3,0xC3,0xDB,0xFF,0xE7,0xC3,0xC3,0xC3,0x00,0x00,0x00,0x00},
        // 88: X
        {0x00,0x00,0xC3,0x66,0x3C,0x18,0x18,0x18,0x3C,0x66,0xC3,0x00,0x00,0x00,0x00,0x00},
        // 89: Y
        {0x00,0x00,0xC3,0xC3,0xC3,0x66,0x3C,0x18,0x18,0x18,0x3C,0x00,0x00,0x00,0x00,0x00},
        // 90: Z
        {0x00,0x00,0xFF,0xC3,0x86,0x0C,0x18,0x30,0x60,0xC1,0xC3,0xFF,0x00,0x00,0x00,0x00},
        // 91: [
        {0x00,0x00,0x3C,0x30,0x30,0x30,0x30,0x30,0x30,0x30,0x30,0x3C,0x00,0x00,0x00,0x00},
        // 92: \ (Backslash)
        {0x00,0x00,0x00,0x00,0x80,0xC0,0x60,0x30,0x18,0x0C,0x06,0x02,0x00,0x00,0x00,0x00},
        // 93: ]
        {0x00,0x00,0x3C,0x0C,0x0C,0x0C,0x0C,0x0C,0x0C,0x0C,0x0C,0x3C,0x00,0x00,0x00,0x00},
        // 94: ^
        {0x18,0x3C,0x66,0xC3,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},
        // 95: _
        {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xFF,0x00,0x00,0x00},
        // 96: `
        {0x30,0x18,0x0C,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},
        // 97: a
        {0x00,0x00,0x00,0x00,0x00,0x3C,0x66,0x06,0x3E,0x66,0x66,0x3F,0x00,0x00,0x00,0x00},
        // 98: b
        {0x00,0x00,0xC0,0xC0,0xC0,0xDC,0xE6,0xC6,0xC6,0xC6,0xE6,0xDC,0x00,0x00,0x00,0x00},
        // 99: c
        {0x00,0x00,0x00,0x00,0x00,0x3C,0x66,0xC0,0xC0,0xC0,0x66,0x3C,0x00,0x00,0x00,0x00},
        // 100: d
        {0x00,0x00,0x06,0x06,0x06,0x3E,0x66,0xC6,0xC6,0xC6,0x66,0x3E,0x00,0x00,0x00,0x00},
        // 101: e
        {0x00,0x00,0x00,0x00,0x00,0x3C,0x66,0xC6,0xFE,0xC0,0x66,0x3C,0x00,0x00,0x00,0x00},
        // 102: f
        {0x00,0x00,0x1C,0x36,0x30,0x30,0x78,0x30,0x30,0x30,0x30,0x78,0x00,0x00,0x00,0x00},
        // 103: g
        {0x00,0x00,0x00,0x00,0x00,0x3E,0x66,0xC6,0xC6,0x66,0x3E,0x06,0x66,0x3C,0x00,0x00},
        // 104: h
        {0x00,0x00,0xC0,0xC0,0xC0,0xDC,0xE6,0xC6,0xC6,0xC6,0xC6,0xC6,0x00,0x00,0x00,0x00},
        // 105: i
        {0x00,0x00,0x18,0x18,0x00,0x38,0x18,0x18,0x18,0x18,0x18,0x3C,0x00,0x00,0x00,0x00},
        // 106: j
        {0x00,0x00,0x06,0x06,0x00,0x0E,0x06,0x06,0x06,0x06,0x06,0x06,0x66,0x3C,0x00,0x00},
        // 107: k
        {0x00,0x00,0xC0,0xC0,0xC0,0xCC,0xD8,0xF0,0xF8,0xD8,0xCC,0xC6,0x00,0x00,0x00,0x00},
        // 108: l
        {0x00,0x00,0x38,0x18,0x18,0x18,0x18,0x18,0x18,0x18,0x18,0x3C,0x00,0x00,0x00,0x00},
        // 109: m
        {0x00,0x00,0x00,0x00,0x00,0xEC,0xFE,0xD6,0xD6,0xD6,0xD6,0xD6,0x00,0x00,0x00,0x00},
        // 110: n
        {0x00,0x00,0x00,0x00,0x00,0xDC,0xE6,0xC6,0xC6,0xC6,0xC6,0xC6,0x00,0x00,0x00,0x00},
        // 111: o
        {0x00,0x00,0x00,0x00,0x00,0x3C,0x66,0xC3,0xC3,0xC3,0x66,0x3C,0x00,0x00,0x00,0x00},
        // 112: p
        {0x00,0x00,0x00,0x00,0x00,0xDC,0xE6,0xC6,0xC6,0xC6,0xE6,0xDC,0xC0,0xC0,0x00,0x00},
        // 113: q
        {0x00,0x00,0x00,0x00,0x00,0x3E,0x66,0xC6,0xC6,0xC6,0x66,0x3E,0x06,0x06,0x00,0x00},
        // 114: r
        {0x00,0x00,0x00,0x00,0x00,0xDE,0xE6,0xC0,0xC0,0xC0,0xC0,0xE0,0x00,0x00,0x00,0x00},
        // 115: s
        {0x00,0x00,0x00,0x00,0x00,0x3C,0x66,0x60,0x3C,0x06,0x66,0x3C,0x00,0x00,0x00,0x00},
        // 116: t
        {0x00,0x00,0x10,0x10,0x10,0x7C,0x30,0x30,0x30,0x30,0x34,0x18,0x00,0x00,0x00,0x00},
        // 117: u
        {0x00,0x00,0x00,0x00,0x00,0xC3,0xC3,0xC3,0xC3,0xC3,0x66,0x3E,0x00,0x00,0x00,0x00},
        // 118: v
        {0x00,0x00,0x00,0x00,0x00,0xC3,0xC3,0xC3,0xC3,0xC3,0x66,0x3C,0x00,0x00,0x00,0x00},
        // 119: w
        {0x00,0x00,0x00,0x00,0x00,0xC3,0xC3,0xC3,0xDB,0xFF,0xE7,0xC3,0x00,0x00,0x00,0x00},
        // 120: x
        {0x00,0x00,0x00,0x00,0x00,0xC3,0x66,0x3C,0x18,0x3C,0x66,0xC3,0x00,0x00,0x00,0x00},
        // 121: y
        {0x00,0x00,0x00,0x00,0x00,0xC3,0xC3,0xC3,0xC3,0x66,0x3C,0x18,0x70,0x00,0x00,0x00},
        // 122: z
        {0x00,0x00,0x00,0x00,0x00,0xFF,0xC3,0x06,0x1C,0x30,0x60,0xFF,0x00,0x00,0x00,0x00},
        // 123: {
        {0x00,0x00,0x0E,0x18,0x18,0x18,0x70,0x18,0x18,0x18,0x18,0x0E,0x00,0x00,0x00,0x00},
        // 124: |
        {0x00,0x00,0x18,0x18,0x18,0x18,0x18,0x18,0x18,0x18,0x18,0x18,0x00,0x00,0x00,0x00},
        // 125: }
        {0x00,0x00,0x70,0x18,0x18,0x18,0x0E,0x18,0x18,0x18,0x18,0x70,0x00,0x00,0x00,0x00},
        // 126: ~
        {0x00,0x00,0x76,0xDC,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},
        // 127: (DEL)
        {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},
    };

    // ============================================================================
    // TEXTURE IMPLEMENTATION
    // ============================================================================
    Texture::Texture() : width(0), height(0) {}
    Texture::Texture(int w, int h) : width(w), height(h) { pixels.resize(w * h, 0); }
    void Texture::setPixel(int x, int y, Color c) { if (x >= 0 && x < width && y >= 0 && y < height) pixels[y * width + x] = c.toInt(); }
    uint32_t Texture::getPixel(int x, int y) const { if (x >= 0 && x < width && y >= 0 && y < height) return pixels[y * width + x]; return 0; }
    void Texture::fill(Color c) { std::fill(pixels.begin(), pixels.end(), c.toInt()); }
    bool Texture::loadFromFile(const std::string& filepath) {
        int w, h, c;
        unsigned char* data = stbi_load(filepath.c_str(), &w, &h, &c, 4);
        if (!data) return false;
        width = w; height = h; pixels.resize(w * h);
        for (int i = 0; i < w * h; ++i) pixels[i] = Color(data[i * 4], data[i * 4 + 1], data[i * 4 + 2], data[i * 4 + 3]).toInt();
        stbi_image_free(data);
        return true;
    }

    // ============================================================================
    // SPRITE IMPLEMENTATION
    // ============================================================================
    Sprite::Sprite() : texture(nullptr), x(0), y(0), scale(1.0f) {}
    Sprite::Sprite(Texture* tex) : texture(tex), x(0), y(0), scale(1.0f) { if (tex) srcRect = Rect(0, 0, tex->width, tex->height); }
    void Sprite::setTexture(Texture* tex) { texture = tex; if (tex) srcRect = Rect(0, 0, tex->width, tex->height); }
    void Sprite::setPosition(float _x, float _y) { x = _x; y = _y; }
    void Sprite::setTextureRect(int rx, int ry, int rw, int rh) { srcRect = Rect(rx, ry, rw, rh); }
    void Sprite::setScale(float s) { scale = s; }

    // ============================================================================
    // WINDOW IMPLEMENTATION
    // ============================================================================

    LRESULT CALLBACK Window::WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
        Window* win = (Window*)GetWindowLongPtr(hwnd, GWLP_USERDATA);
        if (!win) return DefWindowProc(hwnd, uMsg, wParam, lParam);

        switch (uMsg) {
        case WM_CLOSE: win->running = false; return 0;
        case WM_KEYDOWN: win->keys[wParam & 0xFF] = true; return 0;
        case WM_KEYUP: win->keys[wParam & 0xFF] = false; return 0;
        case WM_MOUSEMOVE: win->mouseX = LOWORD(lParam); win->mouseY = HIWORD(lParam); return 0;
        case WM_LBUTTONDOWN: win->mouseLeft = true; win->mouseLeftPressed = true; return 0;
        case WM_LBUTTONUP: win->mouseLeft = false; return 0;
        case WM_RBUTTONDOWN: win->mouseRight = true; return 0;
        case WM_RBUTTONUP: win->mouseRight = false; return 0;
        case WM_MOUSEWHEEL:
            win->mouseScrollDelta += GET_WHEEL_DELTA_WPARAM(wParam);
            return 0;
        }
        return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }

    Window::Window(int w, int h, const std::string& title, bool fullscreen)
        : width(w), height(h), running(true), targetFPS(0), currentDeltaTime(0.0f),
        mouseX(0), mouseY(0), mouseLeft(false), mouseRight(false), mouseLeftPressed(false), mouseScrollDelta(0), activePartitionID(-1)
    {
        SetProcessDPIAware();
        const wchar_t* className = L"TermGLClass";
        std::wstring wtitle(title.begin(), title.end());

        WNDCLASSW wc = {};
        wc.lpfnWndProc = WindowProc;
        wc.hInstance = GetModuleHandleW(NULL);
        wc.lpszClassName = className;
        wc.hCursor = LoadCursor(NULL, IDC_ARROW);
        RegisterClassW(&wc);

        DWORD style = WS_OVERLAPPEDWINDOW | WS_VISIBLE;
        int x = CW_USEDEFAULT, y = CW_USEDEFAULT, winW = width, winH = height;

        if (fullscreen) {
            width = GetSystemMetrics(SM_CXSCREEN);
            height = GetSystemMetrics(SM_CYSCREEN);
            style = WS_OVERLAPPEDWINDOW | WS_VISIBLE | WS_MAXIMIZE;
            x = 0; y = 0; winW = width; winH = height;
        }
        else {
            RECT rect = { 0, 0, width, height };
            AdjustWindowRect(&rect, style, FALSE);
            winW = rect.right - rect.left;
            winH = rect.bottom - rect.top;
            x = (GetSystemMetrics(SM_CXSCREEN) - winW) / 2;
            y = (GetSystemMetrics(SM_CYSCREEN) - winH) / 2;
        }

        hwnd = CreateWindowExW(0, className, wtitle.c_str(), style, x, y, winW, winH, NULL, NULL, GetModuleHandleW(NULL), NULL);
        SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)this);
        hdc = GetDC(hwnd);

        if (fullscreen) {
            ShowWindow(hwnd, SW_MAXIMIZE);
            RECT clientRect; GetClientRect(hwnd, &clientRect);
            width = clientRect.right - clientRect.left;
            height = clientRect.bottom - clientRect.top;
        }

        buffer = new uint32_t[width * height];
        bitmapInfo = {};
        bitmapInfo.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
        bitmapInfo.bmiHeader.biWidth = width;
        bitmapInfo.bmiHeader.biHeight = -height;
        bitmapInfo.bmiHeader.biPlanes = 1;
        bitmapInfo.bmiHeader.biBitCount = 32;
        bitmapInfo.bmiHeader.biCompression = BI_RGB;

        for (int i = 0; i < 256; i++) { keys[i] = false; prevKeys[i] = false; }
        lastFrameTime = std::chrono::steady_clock::now();
    }

    Window::~Window() { delete[] buffer; ReleaseDC(hwnd, hdc); DestroyWindow(hwnd); }

    bool Window::processEvents() {
        auto now = std::chrono::steady_clock::now();
        std::chrono::duration<float> diff = now - lastFrameTime;
        currentDeltaTime = diff.count();
        lastFrameTime = now;
        mouseLeftPressed = false;
        mouseScrollDelta = 0; // Reset scroll delta per frame after processing? 
        // NOTE: Actually, we process messages below, so we should clear it *before* the loop, 
        // but if messages accumulate, we want them. 
        // Standard practice: Reset at start of frame, loop adds to it.
        // Wait, PeekMessage loop is below.

        // Update key states
        for (int i = 0; i < 256; ++i) prevKeys[i] = keys[i];

        MSG msg = {};
        while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
            if (msg.message == WM_QUIT) running = false;
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        return running;
    }

    void Window::setFramerateLimit(int fps) { targetFPS = fps; }
    float Window::getDeltaTime() const { return currentDeltaTime; }

    void Window::display() {
        StretchDIBits(hdc, 0, 0, width, height, 0, 0, width, height, buffer, &bitmapInfo, DIB_RGB_COLORS, SRCCOPY);
        if (targetFPS > 0) {
            float targetFrameTime = 1.0f / targetFPS;
            auto now = std::chrono::steady_clock::now();
            std::chrono::duration<float> frameDuration = now - lastFrameTime;
            if (frameDuration.count() < targetFrameTime) {
                int ms = (int)((targetFrameTime - frameDuration.count()) * 1000);
                if (ms > 1) Sleep(ms);
            }
        }
    }

    // ============================================================================
    // PARTITION MANAGEMENT
    // ============================================================================

    int Window::addPartition(int x, int y, int w, int h, const std::string& title) {
        int id = partitions.size();
        partitions.emplace_back(id, x, y, w, h, title);
        return id;
    }

    void Window::setActivePartition(int id) {
        if (id >= -1 && id < (int)partitions.size()) {
            activePartitionID = id;
            if (id != -1) partitions[id].active = true;
        }
    }

    void Window::drawPartitionFrames() {
        // Temporarily reset active partition to draw globally
        int prevID = activePartitionID;
        activePartitionID = -1;

        for (const auto& p : partitions) {
            // Draw background - Pure black for retro vibes
            fillRect(p.rect.x, p.rect.y, p.rect.w, p.rect.h, Color(0, 0, 0));

            // Draw border
            Color borderC = (prevID == p.id) ? Color::White() : p.borderColor;
            drawRect(p.rect.x, p.rect.y, p.rect.w, p.rect.h, borderC);

            // Draw header bar - Dark grey for retro look
            fillRect(p.rect.x, p.rect.y, p.rect.w, 20, (prevID == p.id) ? Color(40, 40, 40) : Color(20, 20, 20));
            drawText(p.rect.x + 5, p.rect.y + 2, p.title, p.titleColor);
        }

        activePartitionID = prevID;
    }

    void Window::clearPartition(int id, Color color) {
        if (id >= 0 && id < (int)partitions.size()) {
            int prevID = activePartitionID;
            activePartitionID = -1; // Draw globally to clear specific rect
            const auto& p = partitions[id];
            // Clear content area (excluding header)
            fillRect(p.rect.x + 1, p.rect.y + 21, p.rect.w - 2, p.rect.h - 22, color);
            activePartitionID = prevID;
        }
    }

    void Window::transformCoordinates(int& x, int& y) const {
        if (activePartitionID != -1) {
            const auto& p = partitions[activePartitionID];
            x += p.rect.x;
            y += p.rect.y + 20; // Offset by header height
        }
    }

    bool Window::clipCoordinates(int x, int y) const {
        if (activePartitionID != -1) {
            const auto& p = partitions[activePartitionID];
            // Clip to content area (exclude header and borders)
            int minX = p.rect.x + 1;
            int maxX = p.rect.x + p.rect.w - 2;
            int minY = p.rect.y + 21;
            int maxY = p.rect.y + p.rect.h - 2;
            return (x >= minX && x < maxX && y >= minY && y < maxY);
        }
        return (x >= 0 && x < width && y >= 0 && y < height);
    }

    int Window::getWidth() const {
        if (activePartitionID != -1) return partitions[activePartitionID].rect.w;
        return width;
    }

    int Window::getHeight() const {
        if (activePartitionID != -1) return partitions[activePartitionID].rect.h - 20; // Minus header
        return height;
    }

    // ============================================================================
    // DRAWING PRIMITIVES (Context Aware)
    // ============================================================================

    void Window::drawPixel(int x, int y, Color color) {
        transformCoordinates(x, y);
        if (clipCoordinates(x, y)) {
            buffer[x + y * width] = color.toInt();
        }
    }

    void Window::clear(Color color) {
        if (activePartitionID == -1) {
            uint32_t c = color.toInt();
            for (int i = 0; i < width * height; i++) buffer[i] = c;
        }
        else {
            clearPartition(activePartitionID, color);
        }
    }
    void Window::drawBuffer(int x, int y, int w, int h, const uint32_t* data) {
        // 1. Transform coordinates (Handle Partitions)
        int tx = x, ty = y;
        transformCoordinates(tx, ty);

        // 2. Calculate Clipping (Don't draw outside window)
        int minX = 0, maxX = width, minY = 0, maxY = height;
        if (activePartitionID != -1) {
            const auto& p = partitions[activePartitionID];
            minX = p.rect.x + 1; maxX = p.rect.x + p.rect.w - 1;
            minY = p.rect.y + 21; maxY = p.rect.y + p.rect.h - 1;
        }

        int drawX = std::max(minX, tx);
        int drawY = std::max(minY, ty);
        int drawW = std::min(maxX, tx + w) - drawX;
        int drawH = std::min(maxY, ty + h) - drawY;

        if (drawW <= 0 || drawH <= 0) return;

        // 3. Fast Row-by-Row Copy
        // We skip the parts of the image that are clipped off
        int srcOffsetX = drawX - tx;
        int srcOffsetY = drawY - ty;

        for (int row = 0; row < drawH; ++row) {
            // Destination: Screen Buffer
            uint32_t* destPtr = &buffer[(drawY + row) * width + drawX];

            // Source: Video Frame Buffer
            const uint32_t* srcPtr = &data[(srcOffsetY + row) * w + srcOffsetX];

            // The Magic: Copy the whole line in one CPU instruction block
            memcpy(destPtr, srcPtr, drawW * sizeof(uint32_t));
        }
    }


    void Window::drawLine(int x0, int y0, int x1, int y1, Color color) {
        // Bresenham's
        int dx = std::abs(x1 - x0), sx = x0 < x1 ? 1 : -1;
        int dy = -std::abs(y1 - y0), sy = y0 < y1 ? 1 : -1;
        int err = dx + dy, e2;
        while (true) {
            drawPixel(x0, y0, color); // drawPixel handles transform & clip
            if (x0 == x1 && y0 == y1) break;
            e2 = 2 * err;
            if (e2 >= dy) { err += dy; x0 += sx; }
            if (e2 <= dx) { err += dx; y0 += sy; }
        }
    }

    void Window::drawRect(int x, int y, int w, int h, Color color) {
        drawLine(x, y, x + w - 1, y, color);
        drawLine(x, y + h - 1, x + w - 1, y + h - 1, color);
        drawLine(x, y, x, y + h - 1, color);
        drawLine(x + w - 1, y, x + w - 1, y + h - 1, color);
    }

    void Window::fillRect(int x, int y, int w, int h, Color color) {
        // Optimized fill with clipping
        int tx = x, ty = y;
        transformCoordinates(tx, ty);

        // Quick clip check for active partition
        int minX = 0, maxX = width, minY = 0, maxY = height;
        if (activePartitionID != -1) {
            const auto& p = partitions[activePartitionID];
            minX = p.rect.x + 1; maxX = p.rect.x + p.rect.w - 1;
            minY = p.rect.y + 21; maxY = p.rect.y + p.rect.h - 1;
        }

        int startX = std::max(minX, tx);
        int startY = std::max(minY, ty);
        int endX = std::min(maxX, tx + w);
        int endY = std::min(maxY, ty + h);

        if (startX >= endX || startY >= endY) return;

        uint32_t c = color.toInt();
        for (int j = startY; j < endY; j++) {
            for (int i = startX; i < endX; i++) {
                buffer[i + j * width] = c;
            }
        }
    }

    void Window::fillGradientRect(int x, int y, int w, int h, Color c1, Color c2, bool vertical) {
        // Simple pixel-by-pixel for gradients to reuse drawPixel's clipping
        for (int j = 0; j < h; ++j) {
            for (int i = 0; i < w; ++i) {
                float ratio = vertical ? (float)j / h : (float)i / w;
                if (ratio < 0) ratio = 0; if (ratio > 1) ratio = 1;
                uint8_t r = (uint8_t)(c1.r + (c2.r - c1.r) * ratio);
                uint8_t g = (uint8_t)(c1.g + (c2.g - c1.g) * ratio);
                uint8_t b = (uint8_t)(c1.b + (c2.b - c1.b) * ratio);
                uint8_t a = (uint8_t)(c1.a + (c2.a - c1.a) * ratio);
                drawPixel(x + i, y + j, Color(r, g, b, a));
            }
        }
    }

    void Window::drawCircle(int xc, int yc, int r, Color color) {
        int x = 0, y = r, d = 3 - 2 * r;
        auto plot8 = [&](int cx, int cy, int xx, int yy) {
            drawPixel(cx + xx, cy + yy, color); drawPixel(cx - xx, cy + yy, color);
            drawPixel(cx + xx, cy - yy, color); drawPixel(cx - xx, cy - yy, color);
            drawPixel(cx + yy, cy + xx, color); drawPixel(cx - yy, cy + xx, color);
            drawPixel(cx + yy, cy - xx, color); drawPixel(cx - yy, cy - xx, color);
            };
        while (y >= x) {
            plot8(xc, yc, x, y);
            x++;
            if (d > 0) { y--; d = d + 4 * (x - y) + 10; }
            else { d = d + 4 * x + 6; }
        }
    }

    void Window::fillCircle(int xc, int yc, int r, Color color) {
        for (int y = -r; y <= r; y++) {
            for (int x = -r; x <= r; x++) {
                if (x * x + y * y <= r * r) drawPixel(xc + x, yc + y, color);
            }
        }
    }

    void Window::drawTriangle(int x1, int y1, int x2, int y2, int x3, int y3, Color color) {
        drawLine(x1, y1, x2, y2, color);
        drawLine(x2, y2, x3, y3, color);
        drawLine(x3, y3, x1, y1, color);
    }

    void Window::fillTriangle(int x1, int y1, int x2, int y2, int x3, int y3, Color color) {
        auto drawScanLine = [&](int y, int xLeft, int xRight) {
            if (xLeft > xRight) std::swap(xLeft, xRight);
            for (int x = xLeft; x <= xRight; x++) drawPixel(x, y, color);
            };
        if (y1 > y2) { std::swap(x1, x2); std::swap(y1, y2); }
        if (y1 > y3) { std::swap(x1, x3); std::swap(y1, y3); }
        if (y2 > y3) { std::swap(x2, x3); std::swap(y2, y3); }
        int totalHeight = y3 - y1;
        if (totalHeight == 0) return;
        for (int i = 0; i < totalHeight; i++) {
            int y = y1 + i;
            bool secondHalf = i > y2 - y1 || y2 == y1;
            int segmentHeight = secondHalf ? y3 - y2 : y2 - y1;
            float alpha = (float)i / totalHeight;
            float beta = (float)(i - (secondHalf ? y2 - y1 : 0)) / segmentHeight;
            int A = (int)(x1 + (x3 - x1) * alpha);
            int B = secondHalf ? (int)(x2 + (x3 - x2) * beta) : (int)(x1 + (x2 - x1) * beta);
            drawScanLine(y, A, B);
        }
    }

    void Window::drawText(int x, int y, const std::string& text, Color color) {
        int ox = x;
        for (char c : text) {
            if (c == '\n') { y += 20; x = ox; continue; }
            unsigned char glyphIndex = (unsigned char)c;
            if (glyphIndex < 32 || glyphIndex > 127) glyphIndex = 63;
            for (int row = 0; row < 16; row++) {
                unsigned char line = FONT8x16[glyphIndex][row];
                for (int col = 0; col < 8; col++) {
                    if ((line >> (7 - col)) & 1) drawPixel(x + col, y + row, color);
                }
            }
            x += 9;
        }
    }

    bool Window::drawButton(int x, int y, int w, int h, const std::string& text) {
        bool hovering = isMouseHovering(x, y, w, h);
        bool clicked = isButtonClicked(x, y, w, h);

        // Black and grey theme for retro vibes
        Color c1 = hovering ? Color(40, 40, 40) : Color(20, 20, 20);
        Color c2 = hovering ? Color(30, 30, 30) : Color(10, 10, 10);

        if (clicked) {
            c1 = Color(60, 60, 60);
            c2 = Color(40, 40, 40);
        }

        fillGradientRect(x, y, w, h, c1, c2, true);
        drawRect(x, y, w, h, Color(80, 80, 80));

        int textW = text.length() * 9;
        drawText(x + (w - textW) / 2, y + (h - 16) / 2, text, Color::White());

        return clicked;
    }

    // ============================================================================
    // LIST RENDERING (New)
    // ============================================================================
    int Window::drawList(int x, int y, int w, int h, const std::vector<std::string>& items, int& scrollOffset, int itemHeight) {
        int totalHeight = items.size() * itemHeight;

        // Handle Scroll Inputs
        if (isMouseHovering(x, y, w, h)) {
            scrollOffset -= mouseScrollDelta; // Wheel scrolling
        }

        // Clamp Scroll
        int maxScroll = std::max(0, totalHeight - h);
        if (scrollOffset < 0) scrollOffset = 0;
        if (scrollOffset > maxScroll) scrollOffset = maxScroll;

        // Draw List Background - Pure black for retro vibes
        fillRect(x, y, w, h, Color(0, 0, 0));
        drawRect(x, y, w, h, Color(60, 60, 60));

        // Draw Items with manual clipping
        // We use a manual clip approach because 'drawText' relies on 'drawPixel' which respects partition clipping,
        // but we need to clip text specifically to the LIST box, which might be smaller than the partition.
        // For simplicity in this implementation, we will only draw items that overlap the viewport.
        // True pixel-perfect text clipping would require a scissor rect stack in the renderer.
        // As a workaround, we will clear the areas above and below the list after drawing to "clip" it visually 
        // if it spills out (though drawText won't spill out of the partition).
        // Best approach here: Only iterate visible items.

        int startIndex = scrollOffset / itemHeight;
        int endIndex = (scrollOffset + h) / itemHeight + 1;
        if (endIndex > (int)items.size()) endIndex = items.size();

        int clickedIndex = -1;

        for (int i = startIndex; i < endIndex; ++i) {
            int itemY = y + (i * itemHeight) - scrollOffset;

            // Interaction - Dark grey highlight for retro look
            bool hovered = isMouseHovering(x, itemY, w - 15, itemHeight);
            if (hovered && itemY >= y && itemY + itemHeight <= y + h) {
                fillRect(x + 1, itemY, w - 17, itemHeight, Color(30, 30, 30));
                if (isButtonClicked(x, itemY, w - 15, itemHeight)) clickedIndex = i;
            }

            if (itemY + 16 > y && itemY < y + h) {
                int drawY = itemY + (itemHeight - 16) / 2;
                drawText(x + 5, drawY, items[i], Color::White());
            }
        }

        // Draw Scrollbar - Dark grey for retro look
        int barTrackH = h - 2;
        int barH = (totalHeight > 0) ? (h * barTrackH / std::max(h, totalHeight)) : barTrackH;
        if (barH < 10) barH = 10;

        float scrollRatio = (float)scrollOffset / maxScroll;
        if (maxScroll == 0) scrollRatio = 0;
        int barY = y + 1 + (int)(scrollRatio * (barTrackH - barH));

        fillRect(x + w - 12, y, 12, h, Color(15, 15, 15)); // Track - darker
        fillRect(x + w - 10, barY, 8, barH, Color(60, 60, 60)); // Thumb - grey

        return clickedIndex;
    }

    void Window::drawSprite(const Sprite& sprite) {
        if (!sprite.texture) return;
        int destX = (int)sprite.x, destY = (int)sprite.y;
        int destW = (int)(sprite.srcRect.w * sprite.scale);
        int destH = (int)(sprite.srcRect.h * sprite.scale);

        // Simple sprite drawing with drawPixel for clipping support
        for (int y = 0; y < destH; ++y) {
            for (int x = 0; x < destW; ++x) {
                int srcX = sprite.srcRect.x + (x * sprite.srcRect.w / destW);
                int srcY = sprite.srcRect.y + (y * sprite.srcRect.h / destH);
                uint32_t pixel = sprite.texture->getPixel(srcX, srcY);
                if ((pixel & 0xFF000000) != 0) {
                    uint8_t r = (pixel >> 16) & 0xFF;
                    uint8_t g = (pixel >> 8) & 0xFF;
                    uint8_t b = pixel & 0xFF;
                    drawPixel(destX + x, destY + y, Color(r, g, b));
                }
            }
        }
    }

    // ============================================================================
    // INPUT IMPLEMENTATION
    // ============================================================================
    bool Window::isKeyDown(char key) const { return keys[static_cast<unsigned char>(key)]; }
    bool Window::isKeyPressed(char key) const { return keys[static_cast<unsigned char>(key)] && !prevKeys[static_cast<unsigned char>(key)]; }
    bool Window::isMouseLeftDown() const { return mouseLeft; }
    bool Window::isMouseRightDown() const { return mouseRight; }

    Vec2 Window::getMousePos() const {
        int x = mouseX, y = mouseY;
        if (activePartitionID != -1) {
            const auto& p = partitions[activePartitionID];
            x -= p.rect.x;
            y -= (p.rect.y + 20);
        }
        return Vec2(x, y);
    }

    int Window::getMouseScrollDelta() const { return mouseScrollDelta; }

    bool Window::isMouseHovering(int x, int y, int w, int h) const {
        Vec2 mp = getMousePos();
        return (mp.x >= x && mp.x < x + w && mp.y >= y && mp.y < y + h);
    }

    bool Window::isButtonClicked(int x, int y, int w, int h) const {
        return isMouseHovering(x, y, w, h) && mouseLeftPressed;
    }

} // namespace termgl