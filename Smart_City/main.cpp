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
int main() {
    CitySimulator simulator;
    simulator.run();
    return 0;
}


//int main() {
//    // 1. Create Window
//    termgl::Window window(800, 600, "TermGL Feature Showcase");
//    window.setFramerateLimit(60);
//
//    // 2. Load Texture (stb_image integration test)
//    termgl::Texture spriteTexture;
//    std::string filename = "test.png";
//
//    // Attempt to load. If it fails, generate a fallback so the demo works.
//    if (!spriteTexture.loadFromFile(filename)) {
//        std::cout << "[Demo] '" << filename << "' not found. Generating fallback texture..." << std::endl;
//
//        // Fallback: Create a 32x32 checkerboard
//        spriteTexture = termgl::Texture(32, 32);
//        for (int y = 0; y < 32; y++) {
//            for (int x = 0; x < 32; x++) {
//                bool isWhite = ((x / 4) + (y / 4)) % 2 == 0;
//                spriteTexture.setPixel(x, y, isWhite ? termgl::Color::White() : termgl::Color::Red());
//            }
//        }
//    }
//    else {
//        std::cout << "[Demo] Successfully loaded '" << filename << "'!" << std::endl;
//    }
//
//    // 3. Create Sprite from Texture
//    termgl::Sprite playerSprite(&spriteTexture);
//    playerSprite.setPosition(350, 250); // Start near center
//    playerSprite.setScale(2.0f);        // Test scaling
//
//    // Animation vars
//    float timeAccumulator = 0.0f;
//
//    // Main Loop
//    while (window.processEvents()) {
//        float dt = window.getDeltaTime();
//        timeAccumulator += dt;
//
//        // --- Input Handling ---
//        float speed = 300.0f * dt;
//        if (window.isKeyDown(VK_LEFT))  playerSprite.x -= speed;
//        if (window.isKeyDown(VK_RIGHT)) playerSprite.x += speed;
//        if (window.isKeyDown(VK_UP))    playerSprite.y -= speed;
//        if (window.isKeyDown(VK_DOWN))  playerSprite.y += speed;
//
//        // --- Rendering ---
//        window.clear(termgl::Color(25, 25, 30)); // Dark background
//
//        // Test 1: Geometry & Animation
//        // Draw a satellite orbiting a point
//        int centerX = 150, centerY = 150;
//        int radius = 60;
//        int satX = centerX + (int)(cos(timeAccumulator * 2.0f) * radius);
//        int satY = centerY + (int)(sin(timeAccumulator * 2.0f) * radius);
//
//        window.drawCircle(centerX, centerY, radius, termgl::Color::Grey());       // Orbit path
//        window.fillCircle(centerX, centerY, 10, termgl::Color::Yellow());         // Sun
//        window.drawLine(centerX, centerY, satX, satY, termgl::Color(50, 50, 50)); // Tether
//        window.fillCircle(satX, satY, 5, termgl::Color::Blue());                  // Planet
//
//        // Test 2: Rectangles
//        window.drawRect(600, 50, 100, 50, termgl::Color::Green());
//        window.fillRect(610, 60, (int)(fabs(sin(timeAccumulator)) * 80), 30, termgl::Color(0, 255, 0, 100));
//
//        // Test 3: Sprite Rendering
//        window.drawSprite(playerSprite);
//
//        // Test 4: Text Rendering
//        int fps = (dt > 0) ? (int)(1.0f / dt) : 0;
//        window.drawText(10, 10, "TermGL Showcase", termgl::Color::White());
//        window.drawText(10, 25, "FPS: " + std::to_string(fps), termgl::Color::Green());
//
//        window.drawText(10, 500, "Controls:", termgl::Color::Grey());
//        window.drawText(20, 515, "- Arrow Keys: Move Sprite", termgl::Color::White());
//
//        if (spriteTexture.width == 32 && spriteTexture.height == 32) {
//            window.drawText(10, 550, "Note: Using fallback texture.", termgl::Color::Red());
//            window.drawText(10, 560, "      Add 'test.png' to load image.", termgl::Color::Red());
//        }
//
//        window.display();
//    }
//
//    return 0;
//}