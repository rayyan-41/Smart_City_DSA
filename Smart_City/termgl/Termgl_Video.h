#pragma once
#ifndef TERMGL_VIDEO_H
#define TERMGL_VIDEO_H

#include "Termgl.h"
#include <string>
#include <cstdio> // For FILE*
#include <chrono>

// Forward declaration for miniaudio engine to avoid including the huge header here
struct ma_engine;
struct ma_sound;

namespace termgl {

    class VideoPlayer {
    private:
        FILE* pipe;             // The pipe connection to FFmpeg
        uint32_t* frameBuffer;  // Buffer for the current frame
        int width, height;      // Video dimensions
        bool isPlaying;

        // Video Properties
        std::string currentVideoPath;
        double duration;        // Total duration in seconds
        double currentTime;     // Current playback position
        double playbackSpeed;   // 1.0 = normal, 2.0 = 2x, etc.
        int baseFPS;            // FPS of the source video

        // Audio Engine
        ma_engine* audioEngine;
        ma_sound* audioSound;
        bool audioInitialized;

        // Internal helpers
        bool openPipe(double startTime);
        double getVideoDuration(const std::string& path);
        void initAudio(const std::string& path);
        void cleanupAudio();

    public:
        VideoPlayer();
        ~VideoPlayer();

        // Starts streaming video directly via FFmpeg pipe
        bool loadVideo(const std::string& videoPath, int w, int h, int targetFPS = 30);

        // Reads the next frame from the pipe.
        // SYNC STRATEGY: We use the audio clock as the master clock.
        // If video is behind audio -> Skip frames or read faster
        // If video is ahead of audio -> Wait
        bool update(float deltaTime);

        // Seek to specific timestamp in seconds
        void seek(double time);

        // Change playback speed (e.g. 0.5, 1.0, 2.0)
        void setSpeed(double speed);

        // Renders the current frame buffer
        void draw(Window& window, int x, int y);

        // Renders overlay controls
        bool drawControls(Window& window, int x, int y, int w, int h);

        // Clean up
        void close();

        double getDuration() const { return duration; }
        double getCurrentTime() const { return currentTime; }
        double getSpeed() const { return playbackSpeed; }
    };
}
#endif