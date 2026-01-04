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
//    termgl::Window window(1280, 720, "TermGL Partitions", true);
//    window.setFramerateLimit(60);
//
//    // Define partitions
//    int sw = window.getWidth();
//    int sh = window.getHeight();
//
//    // Left: Main View (70% width)
//    int mainView = window.addPartition(10, 10, (sw * 0.7) - 20, sh - 20, "City Map");
//
//    // Right: Controls (30% width)
//    int controlView = window.addPartition((sw * 0.7), 10, (sw * 0.3) - 10, (sh / 2) - 20, "Controls");
//
//    // Bottom Right: Debug (Below Controls)
//    int debugView = window.addPartition((sw * 0.7), (sh / 2), (sw * 0.3) - 10, (sh / 2) - 10, "Debug Log");
//
//    float time = 0.0f;
//    int activePID = mainView; // Start focusing on main view
//
//    // List Data
//    std::vector<std::string> logMessages;
//    for (int i = 0; i < 50; ++i) logMessages.push_back("Log Entry #" + std::to_string(i) + " - System Event");
//    int listScroll = 0;
//
//    while (window.processEvents()) {
//        float dt = window.getDeltaTime();
//        time += dt;
//
//        // Switch focus with Tab
//        if (window.isKeyPressed(VK_TAB)) {
//            activePID = (activePID + 1) % 3;
//        }
//
//        // Draw Background (Global)
//        window.setActivePartition(-1);
//        window.clear(termgl::Color(10, 10, 15));
//        window.drawPartitionFrames();
//
//        // 1. Draw Main View
//        window.setActivePartition(mainView);
//        window.clear(termgl::Color(20, 20, 30));
//
//        // Draw "Map" content
//        int mw = window.getWidth();
//        int mh = window.getHeight();
//        for (int i = 0; i < 10; ++i) {
//            int cx = (mw / 2) + cos(time + i) * 100;
//            int cy = (mh / 2) + sin(time + i) * 100;
//            window.fillCircle(cx, cy, 10, termgl::Color(50, 200, 50));
//            window.drawLine(mw / 2, mh / 2, cx, cy, termgl::Color::Grey());
//        }
//        window.drawText(10, 10, "Interactive Map Area", termgl::Color::White());
//        if (window.drawButton(20, mh - 50, 150, 30, "Reset Cam")) {
//            // Logic...
//        }
//
//        // 2. Draw Controls
//        window.setActivePartition(controlView);
//        window.clear(termgl::Color(30, 30, 40));
//        window.drawText(10, 10, "Control Panel", termgl::Color::Yellow());
//
//        static bool optTraffic = true;
//        if (window.drawButton(10, 40, 200, 30, optTraffic ? "Traffic: ON" : "Traffic: OFF")) {
//            optTraffic = !optTraffic;
//        }
//        window.drawButton(10, 80, 200, 30, "Add Bus Route");
//        window.drawButton(10, 120, 200, 30, "Emergency Alert");
//
//        // 3. Draw Debug (With Scrolling List)
//        window.setActivePartition(debugView);
//        window.clear(termgl::Color(10, 10, 10));
//        window.drawText(10, 10, "System Log (Scrollable):", termgl::Color::Green());
//
//        // Render List
//        int debugW = window.getWidth();
//        int debugH = window.getHeight();
//        int clickedItem = window.drawList(10, 35, debugW - 20, debugH - 45, logMessages, listScroll);
//
//        if (clickedItem != -1) {
//            logMessages.push_back("Clicked Item: " + std::to_string(clickedItem));
//            listScroll = 99999; // Auto scroll to bottom
//        }
//
//        window.display();
//    }
//
//    return 0;
//}