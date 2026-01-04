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
#include "termgl/Termgl_Video.h"
#include <iostream>
int main() {
    CitySimulator simulator;
    simulator.run();
    return 0;
}

//int main() {
//    // 1. Settings
//    const int WIDTH = 1280;
//    const int HEIGHT = 720;
//    const std::string VIDEO_FILE = "sample_video.mp4"; // Ensure this file exists!
//
//    // 2. Initialize Window
//    std::cout << "Initializing Window (" << WIDTH << "x" << HEIGHT << ")..." << std::endl;
//    termgl::Window window(WIDTH, HEIGHT, "TermGL Video Player");
//    window.setFramerateLimit(60);
//
//    // 3. Initialize Video Player
//    termgl::VideoPlayer player;
//    bool isVideoPlaying = false;
//    bool videoFinished = false;
//
//    // 4. Main Loop
//    while (window.processEvents()) {
//        window.clear(termgl::Color::Black());
//
//        if (!isVideoPlaying) {
//            // --- START SCREEN ---
//
//            // Draw Play Button in the center
//            int btnW = 200;
//            int btnH = 60;
//            int btnX = (WIDTH - btnW) / 2;
//            int btnY = (HEIGHT - btnH) / 2;
//
//            if (videoFinished) {
//                window.drawText(btnX + 40, btnY - 40, "Video Finished", termgl::Color::White());
//                if (window.drawButton(btnX, btnY, btnW, btnH, "Replay Video")) {
//                    isVideoPlaying = true;
//                    videoFinished = false;
//                }
//            }
//            else {
//                if (window.drawButton(btnX, btnY, btnW, btnH, "Play Video")) {
//                    isVideoPlaying = true;
//                }
//            }
//
//            // Initialization logic happens when switching state
//            if (isVideoPlaying) {
//                std::cout << "Loading video stream: " << VIDEO_FILE << std::endl;
//                if (!player.loadVideo(VIDEO_FILE, WIDTH, HEIGHT, 30)) {
//                    std::cerr << "Failed to load video! Check if 'ffmpeg.exe' is in 'bin/' folder." << std::endl;
//                    isVideoPlaying = false; // Stay on menu if failed
//                }
//            }
//
//        }
//        else {
//            // --- VIDEO PLAYBACK ---
//
//            // Get delta time for smooth playback speed control
//            float dt = window.getDeltaTime();
//
//            // Update: Reads the next frame from the FFmpeg pipe based on time accumulated
//            // Returns false if the video ends or pipe breaks
//            if (!player.update(dt)) {
//                std::cout << "Video finished." << std::endl;
//                isVideoPlaying = false;
//                videoFinished = true;
//                player.close(); // Cleanup resources
//            }
//            else {
//                // Draw: Blits the raw pixels directly to the screen
//                player.draw(window, 0, 0);
//
//                // Draw Controls: Overlay progress bar and speed controls
//                player.drawControls(window, 0, 0, WIDTH, HEIGHT);
//            }
//        }
//
//        // Display frame
//        window.display();
//    }
//
//    return 0;
//}