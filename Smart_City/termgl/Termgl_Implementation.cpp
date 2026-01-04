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
    // INTERNAL FONT DATA
    // ============================================================================

    static const unsigned char FONT5x8[128][8] = {
        {0}, {0}, {0}, {0}, {0}, {0}, {0}, {0}, {0}, {0}, {0}, {0}, {0}, {0}, {0}, {0},
        {0}, {0}, {0}, {0}, {0}, {0}, {0}, {0}, {0}, {0}, {0}, {0}, {0}, {0}, {0}, {0},
        {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00}, // SPACE
        {0x04,0x04,0x04,0x04,0x04,0x00,0x04,0x00}, {0x0A,0x0A,0x0A,0x00,0x00,0x00,0x00,0x00},
        {0x0A,0x0A,0x1F,0x0A,0x1F,0x0A,0x0A,0x00}, {0x04,0x1F,0x05,0x0E,0x14,0x1F,0x04,0x00},
        {0x18,0x19,0x02,0x04,0x08,0x13,0x03,0x00}, {0x0C,0x12,0x14,0x08,0x15,0x12,0x0D,0x00},
        {0x0C,0x04,0x08,0x00,0x00,0x00,0x00,0x00}, {0x02,0x04,0x08,0x08,0x08,0x04,0x02,0x00},
        {0x08,0x04,0x02,0x02,0x02,0x04,0x08,0x00}, {0x00,0x04,0x15,0x0E,0x15,0x04,0x00,0x00},
        {0x00,0x04,0x04,0x1F,0x04,0x04,0x00,0x00}, {0x00,0x00,0x00,0x00,0x00,0x0C,0x04,0x08},
        {0x00,0x00,0x00,0x1F,0x00,0x00,0x00,0x00}, {0x00,0x00,0x00,0x00,0x00,0x0C,0x0C,0x00},
        {0x00,0x01,0x02,0x04,0x08,0x10,0x00,0x00}, {0x0E,0x11,0x13,0x15,0x19,0x11,0x0E,0x00},
        {0x04,0x0C,0x04,0x04,0x04,0x04,0x0E,0x00}, {0x0E,0x11,0x01,0x02,0x04,0x08,0x1F,0x00},
        {0x1F,0x02,0x04,0x02,0x01,0x11,0x0E,0x00}, {0x02,0x06,0x0A,0x12,0x1F,0x02,0x02,0x00},
        {0x1F,0x10,0x1E,0x01,0x01,0x11,0x0E,0x00}, {0x06,0x08,0x10,0x1E,0x11,0x11,0x0E,0x00},
        {0x1F,0x01,0x02,0x04,0x08,0x08,0x08,0x00}, {0x0E,0x11,0x11,0x0E,0x11,0x11,0x0E,0x00},
        {0x0E,0x11,0x11,0x0F,0x01,0x02,0x0C,0x00}, {0x00,0x0C,0x0C,0x00,0x0C,0x0C,0x00,0x00},
        {0x00,0x0C,0x0C,0x00,0x0C,0x04,0x08,0x00}, {0x02,0x04,0x08,0x10,0x08,0x04,0x02,0x00},
        {0x00,0x00,0x1F,0x00,0x1F,0x00,0x00,0x00}, {0x08,0x04,0x02,0x01,0x02,0x04,0x08,0x00},
        {0x0E,0x11,0x01,0x02,0x04,0x00,0x04,0x00}, {0x0E,0x11,0x01,0x0D,0x15,0x15,0x0E,0x00},
        {0x0E,0x11,0x11,0x1F,0x11,0x11,0x11,0x00}, {0x1E,0x11,0x11,0x1E,0x11,0x11,0x1E,0x00},
        {0x0E,0x11,0x10,0x10,0x10,0x11,0x0E,0x00}, {0x1E,0x11,0x11,0x11,0x11,0x11,0x1E,0x00},
        {0x1F,0x10,0x10,0x1E,0x10,0x10,0x1F,0x00}, {0x1F,0x10,0x10,0x1E,0x10,0x10,0x10,0x00},
        {0x0E,0x11,0x10,0x13,0x11,0x11,0x0E,0x00}, {0x11,0x11,0x11,0x1F,0x11,0x11,0x11,0x00},
        {0x0E,0x04,0x04,0x04,0x04,0x04,0x0E,0x00}, {0x07,0x02,0x02,0x02,0x02,0x12,0x0C,0x00},
        {0x11,0x12,0x14,0x18,0x14,0x12,0x11,0x00}, {0x10,0x10,0x10,0x10,0x10,0x10,0x1F,0x00},
        {0x11,0x1B,0x15,0x15,0x11,0x11,0x11,0x00}, {0x11,0x11,0x19,0x15,0x13,0x11,0x11,0x00},
        {0x0E,0x11,0x11,0x11,0x11,0x11,0x0E,0x00}, {0x1E,0x11,0x11,0x1E,0x10,0x10,0x10,0x00},
        {0x0E,0x11,0x11,0x11,0x15,0x12,0x0D,0x00}, {0x1E,0x11,0x11,0x1E,0x14,0x12,0x11,0x00},
        {0x0E,0x11,0x10,0x0E,0x01,0x11,0x0E,0x00}, {0x1F,0x04,0x04,0x04,0x04,0x04,0x04,0x00},
        {0x11,0x11,0x11,0x11,0x11,0x11,0x0E,0x00}, {0x11,0x11,0x11,0x11,0x11,0x0A,0x04,0x00},
        {0x11,0x11,0x11,0x15,0x15,0x15,0x0A,0x00}, {0x11,0x11,0x0A,0x04,0x0A,0x11,0x11,0x00},
        {0x11,0x11,0x11,0x0A,0x04,0x04,0x04,0x00}, {0x1F,0x01,0x02,0x04,0x08,0x10,0x1F,0x00},
        {0x0E,0x08,0x08,0x08,0x08,0x08,0x0E,0x00}, {0x00,0x10,0x08,0x04,0x02,0x01,0x00,0x00},
        {0x0E,0x02,0x02,0x02,0x02,0x02,0x0E,0x00}, {0x04,0x0A,0x11,0x00,0x00,0x00,0x00,0x00},
        {0x00,0x00,0x00,0x00,0x00,0x00,0x1F,0x00}, {0x08,0x04,0x02,0x00,0x00,0x00,0x00,0x00},
        {0x00,0x00,0x0E,0x01,0x0F,0x11,0x0F,0x00}, {0x10,0x10,0x16,0x19,0x11,0x11,0x1E,0x00},
        {0x00,0x00,0x0E,0x10,0x10,0x11,0x0E,0x00}, {0x01,0x01,0x0D,0x13,0x11,0x11,0x0F,0x00},
        {0x00,0x00,0x0E,0x11,0x1F,0x10,0x0E,0x00}, {0x0C,0x12,0x10,0x1E,0x10,0x10,0x10,0x00},
        {0x00,0x00,0x0E,0x11,0x0F,0x01,0x0E,0x00}, {0x10,0x10,0x16,0x19,0x11,0x11,0x11,0x00},
        {0x04,0x00,0x0C,0x04,0x04,0x04,0x0E,0x00}, {0x02,0x00,0x06,0x02,0x02,0x12,0x0C,0x00},
        {0x10,0x10,0x12,0x14,0x18,0x14,0x12,0x00}, {0x0C,0x04,0x04,0x04,0x04,0x04,0x0E,0x00},
        {0x00,0x00,0x1A,0x15,0x15,0x11,0x11,0x00}, {0x00,0x00,0x16,0x19,0x11,0x11,0x11,0x00},
        {0x00,0x00,0x0E,0x11,0x11,0x11,0x0E,0x00}, {0x00,0x00,0x1E,0x11,0x1E,0x10,0x10,0x00},
        {0x00,0x00,0x0D,0x13,0x0F,0x01,0x01,0x00}, {0x00,0x00,0x16,0x19,0x10,0x10,0x10,0x00},
        {0x00,0x00,0x0E,0x10,0x0E,0x01,0x1E,0x00}, {0x00,0x10,0x1E,0x10,0x10,0x11,0x0E,0x00},
        {0x00,0x00,0x11,0x11,0x11,0x13,0x0D,0x00}, {0x00,0x00,0x11,0x11,0x11,0x0A,0x04,0x00},
        {0x00,0x00,0x11,0x11,0x15,0x15,0x0A,0x00}, {0x00,0x00,0x11,0x0A,0x04,0x0A,0x11,0x00},
        {0x00,0x00,0x11,0x11,0x0F,0x01,0x0E,0x00}, {0x00,0x00,0x1F,0x02,0x04,0x08,0x1F,0x00},
        {0x02,0x04,0x04,0x08,0x04,0x04,0x02,0x00}, {0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x00},
        {0x08,0x04,0x04,0x02,0x04,0x04,0x08,0x00}, {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00}
    };

    // ============================================================================
    // TEXTURE IMPLEMENTATION
    // ============================================================================

    Texture::Texture() : width(0), height(0) {}

    Texture::Texture(int w, int h) : width(w), height(h) {
        pixels.resize(w * h, 0);
    }

    void Texture::setPixel(int x, int y, Color c) {
        if (x >= 0 && x < width && y >= 0 && y < height) {
            pixels[y * width + x] = c.toInt();
        }
    }

    uint32_t Texture::getPixel(int x, int y) const {
        if (x >= 0 && x < width && y >= 0 && y < height) {
            return pixels[y * width + x];
        }
        return 0;
    }

    void Texture::fill(Color c) {
        std::fill(pixels.begin(), pixels.end(), c.toInt());
    }

    bool Texture::loadFromFile(const std::string& filepath) {
        int w, h, channels;
        unsigned char* data = stbi_load(filepath.c_str(), &w, &h, &channels, 4);

        if (!data) {
            throw std::runtime_error("Failed to load texture: " + filepath +
                " Reason: " + stbi_failure_reason());
        }

        width = w;
        height = h;
        pixels.resize(width * height);

        // Convert raw bytes (RGBA) to our internal pixel format (BGRA / 0xAARRGGBB)
        for (int i = 0; i < width * height; ++i) {
            unsigned char r = data[i * 4 + 0];
            unsigned char g = data[i * 4 + 1];
            unsigned char b = data[i * 4 + 2];
            unsigned char a = data[i * 4 + 3];

            pixels[i] = Color(r, g, b, a).toInt();
        }

        stbi_image_free(data);
        return true;
    }

    // ============================================================================
    // SPRITE IMPLEMENTATION
    // ============================================================================

    Sprite::Sprite() : texture(nullptr), x(0), y(0), scale(1.0f) {}

    Sprite::Sprite(Texture* tex) : texture(tex), x(0), y(0), scale(1.0f) {
        if (tex) {
            srcRect = Rect(0, 0, tex->width, tex->height);
        }
    }

    void Sprite::setTexture(Texture* tex) {
        texture = tex;
        if (tex) {
            srcRect = Rect(0, 0, tex->width, tex->height);
        }
    }

    void Sprite::setPosition(float _x, float _y) {
        x = _x;
        y = _y;
    }

    void Sprite::setTextureRect(int rx, int ry, int rw, int rh) {
        srcRect = Rect(rx, ry, rw, rh);
    }

    void Sprite::setScale(float s) {
        scale = s;
    }

    // ============================================================================
    // WINDOW IMPLEMENTATION
    // ============================================================================

    LRESULT CALLBACK Window::WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
        Window* win = (Window*)GetWindowLongPtr(hwnd, GWLP_USERDATA);
        if (!win) return DefWindowProc(hwnd, uMsg, wParam, lParam);

        switch (uMsg) {
        case WM_DESTROY:
            win->running = false;
            PostQuitMessage(0);
            return 0;
        case WM_KEYDOWN:
            win->keys[wParam & 0xFF] = true;
            return 0;
        case WM_KEYUP:
            win->keys[wParam & 0xFF] = false;
            return 0;
        case WM_MOUSEMOVE:
            win->mouseX = LOWORD(lParam);
            win->mouseY = HIWORD(lParam);
            return 0;
        case WM_LBUTTONDOWN:
            win->mouseLeft = true;
            return 0;
        case WM_LBUTTONUP:
            win->mouseLeft = false;
            return 0;
        case WM_RBUTTONDOWN:
            win->mouseRight = true;
            return 0;
        case WM_RBUTTONUP:
            win->mouseRight = false;
            return 0;
        }
        return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }

    Window::Window(int w, int h, const std::string& title)
        : width(w), height(h), running(true), targetFPS(0), currentDeltaTime(0.0f),
        mouseX(0), mouseY(0), mouseLeft(false), mouseRight(false)
    {
        const wchar_t* className = L"TermGLClass";
        std::wstring wtitle(title.begin(), title.end());

        // Register Class
        WNDCLASSW wc = {};
        wc.lpfnWndProc = WindowProc;
        wc.hInstance = GetModuleHandleW(NULL);
        wc.lpszClassName = className;
        wc.hCursor = LoadCursor(NULL, IDC_ARROW);
        RegisterClassW(&wc);

        // Adjust size to include borders
        RECT rect = { 0, 0, width, height };
        AdjustWindowRect(&rect, WS_OVERLAPPEDWINDOW | WS_VISIBLE, FALSE);

        // Create Window
        hwnd = CreateWindowExW(
            0, className, wtitle.c_str(),
            WS_OVERLAPPEDWINDOW | WS_VISIBLE,
            CW_USEDEFAULT, CW_USEDEFAULT,
            rect.right - rect.left, rect.bottom - rect.top,
            NULL, NULL, GetModuleHandleW(NULL), NULL
        );

        SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)this);
        hdc = GetDC(hwnd);

        // Init Buffer
        buffer = new uint32_t[width * height];

        // Init Bitmap Info
        bitmapInfo = {};
        bitmapInfo.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
        bitmapInfo.bmiHeader.biWidth = width;
        bitmapInfo.bmiHeader.biHeight = -height;
        bitmapInfo.bmiHeader.biPlanes = 1;
        bitmapInfo.bmiHeader.biBitCount = 32;
        bitmapInfo.bmiHeader.biCompression = BI_RGB;

        // Init Input
        for (int i = 0; i < 256; i++) {
            keys[i] = false;
        }

        // Init Timing
        lastFrameTime = std::chrono::steady_clock::now();
    }

    Window::~Window() {
        delete[] buffer;
        ReleaseDC(hwnd, hdc);
        DestroyWindow(hwnd);
    }

    bool Window::processEvents() {
        auto now = std::chrono::steady_clock::now();
        std::chrono::duration<float> diff = now - lastFrameTime;
        currentDeltaTime = diff.count();
        lastFrameTime = now;

        MSG msg = {};
        while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
            if (msg.message == WM_QUIT) running = false;
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        return running;
    }

    void Window::setFramerateLimit(int fps) {
        targetFPS = fps;
    }

    float Window::getDeltaTime() const {
        return currentDeltaTime;
    }

    void Window::display() {
        StretchDIBits(
            hdc, 0, 0, width, height,
            0, 0, width, height,
            buffer, &bitmapInfo,
            DIB_RGB_COLORS, SRCCOPY
        );

        // Cap Framerate
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

    void Window::clear(Color color) {
        uint32_t c = color.toInt();
        for (int i = 0; i < width * height; i++) {
            buffer[i] = c;
        }
    }

    // ============================================================================
    // DRAWING PRIMITIVES
    // ============================================================================

    void Window::drawPixel(int x, int y, Color color) {
        if (x >= 0 && x < width && y >= 0 && y < height) {
            buffer[x + y * width] = color.toInt();
        }
    }

    void Window::drawLine(int x0, int y0, int x1, int y1, Color color) {
        int dx = std::abs(x1 - x0), sx = x0 < x1 ? 1 : -1;
        int dy = -std::abs(y1 - y0), sy = y0 < y1 ? 1 : -1;
        int err = dx + dy, e2;

        while (true) {
            drawPixel(x0, y0, color);
            if (x0 == x1 && y0 == y1) break;
            e2 = 2 * err;
            if (e2 >= dy) { err += dy; x0 += sx; }
            if (e2 <= dx) { err += dx; y0 += sy; }
        }
    }

    void Window::drawRect(int x, int y, int w, int h, Color color) {
        // Horizontal lines
        for (int i = x; i < x + w; i++) {
            drawPixel(i, y, color);
            drawPixel(i, y + h - 1, color);
        }
        // Vertical lines
        for (int j = y; j < y + h; j++) {
            drawPixel(x, j, color);
            drawPixel(x + w - 1, j, color);
        }
    }

    void Window::fillRect(int x, int y, int w, int h, Color color) {
        int startX = std::max(0, x);
        int startY = std::max(0, y);
        int endX = std::min(width, x + w);
        int endY = std::min(height, y + h);

        uint32_t c = color.toInt();

        for (int j = startY; j < endY; j++) {
            for (int i = startX; i < endX; i++) {
                buffer[i + j * width] = c;
            }
        }
    }

    void Window::drawCircle(int xc, int yc, int r, Color color) {
        int x = 0, y = r;
        int d = 3 - 2 * r;

        auto plot8 = [&](int cx, int cy, int xx, int yy) {
            drawPixel(cx + xx, cy + yy, color);
            drawPixel(cx - xx, cy + yy, color);
            drawPixel(cx + xx, cy - yy, color);
            drawPixel(cx - xx, cy - yy, color);
            drawPixel(cx + yy, cy + xx, color);
            drawPixel(cx - yy, cy + xx, color);
            drawPixel(cx + yy, cy - xx, color);
            drawPixel(cx - yy, cy - xx, color);
            };

        while (y >= x) {
            plot8(xc, yc, x, y);
            x++;
            if (d > 0) {
                y--;
                d = d + 4 * (x - y) + 10;
            }
            else {
                d = d + 4 * x + 6;
            }
        }
    }

    void Window::fillCircle(int xc, int yc, int r, Color color) {
        for (int y = -r; y <= r; y++) {
            for (int x = -r; x <= r; x++) {
                if (x * x + y * y <= r * r) {
                    drawPixel(xc + x, yc + y, color);
                }
            }
        }
    }

    void Window::drawText(int x, int y, const std::string& text, Color color) {
        int ox = x;
        for (char c : text) {
            if (c == '\n') {
                y += 10;
                x = ox;
                continue;
            }
            if (c < 0 || c > 127) c = '?';

            for (int row = 0; row < 8; row++) {
                unsigned char line = FONT5x8[(unsigned char)c][row];
                for (int col = 0; col < 5; col++) {
                    if ((line >> (4 - col)) & 1) {
                        drawPixel(x + col * 2, y + row * 2, color);
                        drawPixel(x + col * 2 + 1, y + row * 2, color);
                        drawPixel(x + col * 2, y + row * 2 + 1, color);
                        drawPixel(x + col * 2 + 1, y + row * 2 + 1, color);
                    }
                }
            }
            x += 12;
        }
    }

    // ============================================================================
    // SPRITE RENDERING
    // ============================================================================

    void Window::drawSprite(const Sprite& sprite) {
        if (!sprite.texture) return;

        int destX = static_cast<int>(sprite.x);
        int destY = static_cast<int>(sprite.y);
        int destW = static_cast<int>(sprite.srcRect.w * sprite.scale);
        int destH = static_cast<int>(sprite.srcRect.h * sprite.scale);

        for (int y = 0; y < destH; ++y) {
            for (int x = 0; x < destW; ++x) {
                int srcX = sprite.srcRect.x + (x * sprite.srcRect.w / destW);
                int srcY = sprite.srcRect.y + (y * sprite.srcRect.h / destH);

                uint32_t pixel = sprite.texture->getPixel(srcX, srcY);

                // Alpha check - skip transparent pixels
                if ((pixel & 0xFF000000) != 0) {
                    int drawX = destX + x;
                    int drawY = destY + y;

                    if (drawX >= 0 && drawX < width && drawY >= 0 && drawY < height) {
                        buffer[drawX + drawY * width] = pixel;
                    }
                }
            }
        }
    }

    // ============================================================================
    // INPUT IMPLEMENTATION
    // ============================================================================

    Vec2 Window::getMousePos() const {
        return Vec2(mouseX, mouseY);
    }

    bool Window::isMouseLeftDown() const {
        return mouseLeft;
    }

    bool Window::isMouseRightDown() const {
        return mouseRight;
    }

    bool Window::isKeyDown(char key) const {
        return keys[static_cast<unsigned char>(key)];
    }

} // namespace termgl