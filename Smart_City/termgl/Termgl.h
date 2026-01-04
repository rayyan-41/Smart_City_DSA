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
    // PARTITION STRUCT
    // ============================================================================
    struct Partition {
        int id;
        Rect rect; // x, y, width, height relative to window
        std::string title;
        bool active;
        Color borderColor;
        Color titleColor;
        Color backgroundColor;

        Partition(int _id, int _x, int _y, int _w, int _h, const std::string& _title)
            : id(_id), rect(_x, _y, _w, _h), title(_title), active(false),
            borderColor(60, 60, 60), titleColor(255, 255, 255), backgroundColor(0, 0, 0) {
        }
    };

    // ============================================================================
    // WINDOW CLASS
    // ============================================================================
    class Window {
    public:
        // Updated constructor to support fullscreen mode
        Window(int width, int height, const std::string& title, bool fullscreen = false);
        ~Window();

        // System
        bool processEvents();
        void setFramerateLimit(int fps);
        float getDeltaTime() const;
        void display();

        // Partition Management
        int addPartition(int x, int y, int w, int h, const std::string& title);
        void setActivePartition(int id); // -1 for global window
        void drawPartitionFrames(); // Draws borders and titles for all partitions
        void clearPartition(int id, Color color); // Clears specific partition
        int getActivePartitionID() const { return activePartitionID; }

        // Graphics (Context-aware: draws to active partition)
        void clear(Color color);
        void drawPixel(int x, int y, Color color);
        void drawLine(int x0, int y0, int x1, int y1, Color color);
        void drawRect(int x, int y, int w, int h, Color color);
        void fillRect(int x, int y, int w, int h, Color color);
        void fillGradientRect(int x, int y, int w, int h, Color c1, Color c2, bool vertical);
        void drawCircle(int xc, int yc, int r, Color color);
        void fillCircle(int xc, int yc, int r, Color color);
        void drawTriangle(int x1, int y1, int x2, int y2, int x3, int y3, Color color);
        void fillTriangle(int x1, int y1, int x2, int y2, int x3, int y3, Color color);
        void drawText(int x, int y, const std::string& text, Color color);
        void drawBuffer(int x, int y, int w, int h, const uint32_t* data);

        // UI Components
        bool drawButton(int x, int y, int w, int h, const std::string& text);
        // New: Draw a scrollable list. Returns index of clicked item (-1 if none)
        // 'scrollOffset' is in pixels. 'itemHeight' is usually ~20.
        int drawList(int x, int y, int w, int h, const std::vector<std::string>& items, int& scrollOffset, int itemHeight = 20);

        // Sprite Drawing
        void drawSprite(const Sprite& sprite);

        // Input
        // Updated: Using 'int' key allows passing VK_ codes (e.g., VK_SHIFT, VK_CONTROL)
        bool isKeyDown(int key) const;
        bool isKeyPressed(int key) const; // Single press check

        // Input Helpers for Key Combinations / Modifiers
        bool isControlDown() const;
        bool isShiftDown() const;
        bool isAltDown() const;

        bool isMouseLeftDown() const;
        bool isMouseRightDown() const;
        Vec2 getMousePos() const; // Relative to active partition!

        // Scrolling Support
        int getMouseScrollDelta() const;  // Vertical Scroll (Wheel)
        int getMouseHScrollDelta() const; // Horizontal Scroll (Tilt/Pad)

        // UI Helpers
        bool isMouseHovering(int x, int y, int w, int h) const;
        bool isButtonClicked(int x, int y, int w, int h) const;

        // Getters for dimensions (returns partition size if active)
        int getWidth() const;
        int getHeight() const;

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

        // Partitions
        std::vector<Partition> partitions;
        int activePartitionID; // -1 = Global/None

        // Helper to transform coordinates based on active partition
        void transformCoordinates(int& x, int& y) const;
        // Helper to check if point is within active partition bounds
        bool clipCoordinates(int x, int y) const;

        // Timing
        int targetFPS;
        float currentDeltaTime;
        std::chrono::steady_clock::time_point lastFrameTime;

        // Input State
        int mouseX, mouseY;
        bool mouseLeft, mouseRight;
        bool mouseLeftPressed;

        // Scroll Deltas
        int mouseScrollDelta;  // Vertical
        int mouseHScrollDelta; // Horizontal

        bool keys[256];
        bool prevKeys[256];

        // Internal Helpers
        static LRESULT CALLBACK WindowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
    };

} // namespace termgl

#endif // TERMGL_H