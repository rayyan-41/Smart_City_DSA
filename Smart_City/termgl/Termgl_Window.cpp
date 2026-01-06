#include "Termgl.h"
#include <stdexcept>
#include <chrono>

namespace termgl {

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
        case WM_MOUSEHWHEEL: // Horizontal Scroll Support
            win->mouseHScrollDelta += GET_WHEEL_DELTA_WPARAM(wParam);
            return 0;
        }
        return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }

    Window::Window(int w, int h, const std::string& title, bool fullscreen)
        : width(w), height(h), running(true), targetFPS(0), currentDeltaTime(0.0f),
        mouseX(0), mouseY(0), mouseLeft(false), mouseRight(false), mouseLeftPressed(false),
        mouseScrollDelta(0), mouseHScrollDelta(0), activePartitionID(-1)
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

        // Reset deltas for the new frame
        mouseScrollDelta = 0;
        mouseHScrollDelta = 0;

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
}
