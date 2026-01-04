#pragma once
#ifndef TERMGL_H
#define TERMGL_H

// Prevent Windows min/max macros from conflicting with std::min/std::max
#ifndef NOMINMAX
#define NOMINMAX
#endif

// Reduce the number of Windows headers included to avoid conflicts
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <string>
#include <vector>
#include <cstdint>
#include <windows.h>
#include <chrono>

// Include definitions for Vec2, Rect, Color
#include "Termgl_Defs.h"

namespace termgl {

    // ============================================================================
    // TEXTURE CLASS (Handles Image Data/Bitmaps)
    // ============================================================================
    class Texture {
    public:
        int width, height;
        std::vector<uint32_t> pixels;

        Texture();
        Texture(int w, int h);

        void setPixel(int x, int y, Color c);
        uint32_t getPixel(int x, int y) const;
        void fill(Color c);
        bool loadFromFile(const std::string& filepath);
    };

    // ============================================================================
    // SPRITE STRUCT
    // ============================================================================
    struct Sprite {
        Texture* texture;
        Rect srcRect;
        float x, y;
        float scale;

        Sprite();
        Sprite(Texture* tex);

        void setTexture(Texture* tex);
        void setPosition(float _x, float _y);
        void setTextureRect(int x, int y, int w, int h);
        void setScale(float s);
    };

    // ============================================================================
    // WINDOW CLASS
    // ============================================================================
    class Window {
    public:
        Window(int width, int height, const std::string& title);
        ~Window();

        // System
        bool processEvents();
        void setFramerateLimit(int fps);
        float getDeltaTime() const;
        void display();

        // Graphics
        void clear(Color color);
        void drawPixel(int x, int y, Color color);
        void drawLine(int x0, int y0, int x1, int y1, Color color);
        void drawRect(int x, int y, int w, int h, Color color);
        void fillRect(int x, int y, int w, int h, Color color);
        void drawCircle(int xc, int yc, int r, Color color);
        void fillCircle(int xc, int yc, int r, Color color);
        void drawText(int x, int y, const std::string& text, Color color);

        // Sprite Drawing
        void drawSprite(const Sprite& sprite);

        // Input
        bool isKeyDown(char key) const;
        bool isMouseLeftDown() const;
        bool isMouseRightDown() const;
        Vec2 getMousePos() const;

    private:
        // Window Handle & Context
        HWND hwnd;
        HDC hdc;

        // Software Render Buffer
        uint32_t* buffer;
        BITMAPINFO bitmapInfo;
        int width;
        int height;
        bool running;

        // Timing
        int targetFPS;
        float currentDeltaTime;
        std::chrono::steady_clock::time_point lastFrameTime;

        // Input State
        int mouseX, mouseY;
        bool mouseLeft, mouseRight;
        bool keys[256];

        // Internal Helpers
        static LRESULT CALLBACK WindowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
    };

} // namespace termgl

#endif // TERMGL_H