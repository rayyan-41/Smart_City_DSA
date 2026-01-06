#include "Termgl.h"
#include <algorithm>

// stb_image.h is already included with STB_IMAGE_IMPLEMENTATION in Termgl_Implementation.cpp
// We just need the function declarations here
#ifndef STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#endif

namespace termgl {

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

}
