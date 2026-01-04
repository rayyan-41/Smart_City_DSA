// Prevent Windows min/max macros and std::byte conflicts
#ifndef NOMINMAX
#define NOMINMAX
#endif
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

//using namespace std;
#include "source/Simulator/CitySimulator.h"
#include "termgl/Termgl.h"
#include <iostream>
//int main() {
//    CitySimulator simulator;
//    simulator.run();
//    return 0;
//}


bool DrawButton(termgl::Window& win, int x, int y, int w, int h, const std::string& label) {
    bool hovering = win.isMouseHovering(x, y, w, h);
    bool clicked = win.isButtonClicked(x, y, w, h);

    // Gradient background for the button
    termgl::Color c1 = hovering ? termgl::Color(120, 120, 255) : termgl::Color(60, 60, 180);
    termgl::Color c2 = hovering ? termgl::Color(80, 80, 200) : termgl::Color(30, 30, 100);
    if (clicked) {
        c1 = termgl::Color(220, 220, 255);
        c2 = termgl::Color(180, 180, 255);
    }

    win.fillGradientRect(x, y, w, h, c1, c2, true);
    win.drawRect(x, y, w, h, termgl::Color::White());

    // Simple centering for text (approximate for 8x16 font)
    int textWidth = label.length() * 9; // 9 pixels per char
    int textX = x + (w - textWidth) / 2;
    int textY = y + (h - 16) / 2; // 16 pixels height
    win.drawText(textX, textY, label, termgl::Color::White());

    return clicked;
}

int main() {
    // 1. Create Fullscreen Window
    // The library now automatically detects screen resolution and handles DPI
    termgl::Window window(800, 600, "TermGL Feature Test", true);
    window.setFramerateLimit(60);

    // 2. Load Texture (or fallback)
    termgl::Texture spriteTexture;
    std::string filename = "test.png";

    if (!spriteTexture.loadFromFile(filename)) {
        // Generate a fallback checkerboard texture if file is missing
        spriteTexture = termgl::Texture(64, 64);
        for (int y = 0; y < 64; y++) {
            for (int x = 0; x < 64; x++) {
                bool isWhite = ((x / 8) + (y / 8)) % 2 == 0;
                spriteTexture.setPixel(x, y, isWhite ? termgl::Color::White() : termgl::Color(255, 50, 50));
            }
        }
    }

    // 3. Create Sprite
    termgl::Sprite playerSprite(&spriteTexture);
    playerSprite.setPosition(static_cast<float>(window.getWidth()) / 2, static_cast<float>(window.getHeight()) / 2);
    playerSprite.setScale(2.0f);

    float timeAccumulator = 0.0f;
    int clickCount = 0;
    std::string statusMessage = "Hover over elements...";

    // Main Loop
    while (window.processEvents()) {
        float dt = window.getDeltaTime();
        timeAccumulator += dt;

        // --- Update Logic ---
        float speed = 300.0f * dt;
        if (window.isKeyDown(VK_LEFT))  playerSprite.x -= speed;
        if (window.isKeyDown(VK_RIGHT)) playerSprite.x += speed;
        if (window.isKeyDown(VK_UP))    playerSprite.y -= speed;
        if (window.isKeyDown(VK_DOWN))  playerSprite.y += speed;

        // --- Rendering ---
        window.clear(termgl::Color(20, 20, 25)); // Dark background

        int screenW = window.getWidth();
        int screenH = window.getHeight();

        // Feature 1: Gradient Background Panel
        window.fillGradientRect(0, 0, screenW, 80, termgl::Color(40, 40, 60), termgl::Color(20, 20, 25), true);

        // Feature 2: Geometric Primitives
        int cx = screenW / 4;
        int cy = screenH / 3;
        int radius = 50 + (int)(sin(timeAccumulator * 2.0f) * 20);

        window.drawCircle(cx, cy, radius, termgl::Color::Green());
        window.fillCircle(cx, cy, 10, termgl::Color::Yellow());
        window.drawRect(cx - 100, cy + 100, 200, 50, termgl::Color::Red());
        window.drawLine(cx, cy, (int)playerSprite.x, (int)playerSprite.y, termgl::Color(100, 100, 100));

        // Feature 3: Triangle (New)
        int tx = cx + 200;
        int ty = cy;
        // Rotating Triangle
        float angle = timeAccumulator;
        int t1x = tx + (int)(cos(angle) * 50);
        int t1y = ty + (int)(sin(angle) * 50);
        int t2x = tx + (int)(cos(angle + 2.09) * 50);
        int t2y = ty + (int)(sin(angle + 2.09) * 50);
        int t3x = tx + (int)(cos(angle + 4.18) * 50);
        int t3y = ty + (int)(sin(angle + 4.18) * 50);

        window.fillTriangle(t1x, t1y, t2x, t2y, t3x, t3y, termgl::Color(255, 165, 0));
        window.drawTriangle(t1x, t1y, t2x, t2y, t3x, t3y, termgl::Color::White());


        // Feature 4: Text Rendering (Now with High-Res Font)
        window.drawText(10, 10, "TermGL Fullscreen Test (High-Res Font)", termgl::Color::White());
        window.drawText(10, 30, "Resolution: " + std::to_string(screenW) + "x" + std::to_string(screenH), termgl::Color::Grey());
        window.drawText(10, 50, "Status: " + statusMessage, termgl::Color::Yellow());
        window.drawText(10, 70, "Clicks: " + std::to_string(clickCount), termgl::Color::Cyan());

        // Feature 5: Sprite Rendering
        window.drawSprite(playerSprite);

        // Feature 6: Interactive Buttons (Hover & Click with Gradients)
        int btnW = 150;
        int btnH = 40;
        int btnX = screenW - btnW - 50;
        int btnY = 50;

        if (DrawButton(window, btnX, btnY, btnW, btnH, "Click Me!")) {
            clickCount++;
            statusMessage = "Button Clicked!";
        }

        if (DrawButton(window, btnX, btnY + 60, btnW, btnH, "Exit")) {
            // Manually break loop to test control return
            break;
        }

        // Feature 7: Mouse Hover Detection on arbitrary zones
        int zoneX = 50;
        int zoneY = screenH - 150;
        int zoneW = 200;
        int zoneH = 100;

        bool inZone = window.isMouseHovering(zoneX, zoneY, zoneW, zoneH);
        termgl::Color zoneColor = inZone ? termgl::Color(0, 100, 0) : termgl::Color(50, 50, 50);
        window.fillRect(zoneX, zoneY, zoneW, zoneH, zoneColor);
        window.drawRect(zoneX, zoneY, zoneW, zoneH, termgl::Color::Green());
        window.drawText(zoneX + 10, zoneY + 10, inZone ? "Mouse Inside!" : "Hover Zone", termgl::Color::White());

        // Display Frame
        window.display();
    }

    std::cout << "Exited graphical window. Returning to console control." << std::endl;
    return 0;
}