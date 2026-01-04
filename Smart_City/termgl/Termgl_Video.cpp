#include "Termgl_Video.h"
#include <iostream>
#include <string>
#include <filesystem> 
#include <sstream>
#include <cmath>
#include <vector>

// Define miniaudio implementation ONLY here
#define MINIAUDIO_IMPLEMENTATION
#include "miniaudio.h"

// Windows-specific for _popen/_pclose
#ifdef _WIN32
#define POPEN _popen
#define PCLOSE _pclose
#else
#define POPEN popen
#define PCLOSE pclose
#endif

namespace termgl {

    VideoPlayer::VideoPlayer()
        : pipe(nullptr), frameBuffer(nullptr), width(0), height(0), isPlaying(false),
        baseFPS(30), duration(0.0), currentTime(0.0), playbackSpeed(1.0),
        audioEngine(nullptr), audioSound(nullptr), audioInitialized(false) {
    }

    VideoPlayer::~VideoPlayer() {
        close();
    }

    // Helper to find ffmpeg executable
    static std::string getFFmpegPath() {
        if (std::filesystem::exists("bin/ffmpeg.exe")) return "bin\\ffmpeg.exe";
        if (std::filesystem::exists("ffmpeg.exe")) return "ffmpeg.exe";
        return "ffmpeg"; // Hope it's in PATH
    }

    double VideoPlayer::getVideoDuration(const std::string& path) {
        std::string cmd = getFFmpegPath() + " -i \"" + path + "\" 2>&1";
        FILE* p = POPEN(cmd.c_str(), "r");
        if (!p) return 0.0;

        char buffer[128];
        std::string output = "";
        while (fgets(buffer, sizeof(buffer), p)) {
            output += buffer;
        }
        PCLOSE(p);

        size_t pos = output.find("Duration: ");
        if (pos != std::string::npos) {
            std::string timeStr = output.substr(pos + 10, 11);
            int h, m;
            float s;
            if (sscanf_s(timeStr.c_str(), "%d:%d:%f", &h, &m, &s) == 3) {
                return h * 3600.0 + m * 60.0 + s;
            }
        }
        return 0.0;
    }

    void VideoPlayer::initAudio(const std::string& path) {
        cleanupAudio();

        audioEngine = new ma_engine;
        audioSound = new ma_sound;

        if (ma_engine_init(NULL, audioEngine) != MA_SUCCESS) {
            std::cerr << "Failed to initialize audio engine." << std::endl;
            return;
        }

        // Load the sound in streaming mode (better for large video files)
        if (ma_sound_init_from_file(audioEngine, path.c_str(), MA_SOUND_FLAG_STREAM, NULL, NULL, audioSound) != MA_SUCCESS) {
            std::cerr << "Failed to load audio from video file." << std::endl;
            return;
        }

        audioInitialized = true;
        ma_sound_start(audioSound);
    }

    void VideoPlayer::cleanupAudio() {
        if (audioInitialized) {
            ma_sound_uninit(audioSound);
            ma_engine_uninit(audioEngine);
            delete audioSound;
            delete audioEngine;
            audioInitialized = false;
        }
        audioEngine = nullptr;
        audioSound = nullptr;
    }

    bool VideoPlayer::openPipe(double startTime) {
        if (pipe) PCLOSE(pipe);

        std::string ffmpegCmd = getFFmpegPath();
        std::string seekStr = (startTime > 0) ? " -ss " + std::to_string(startTime) : "";
        std::string filter = "scale=" + std::to_string(width) + ":" + std::to_string(height) +
            ":force_original_aspect_ratio=decrease,pad=" + std::to_string(width) +
            ":" + std::to_string(height) + ":(ow-iw)/2:(oh-ih)/2,setsar=1";

        std::string cmd = ffmpegCmd + " -loglevel quiet -hide_banner" + seekStr + " -i \"" + currentVideoPath +
            "\" -f image2pipe -pix_fmt bgra -vcodec rawvideo -vf \"" + filter +
            "\" -r " + std::to_string(baseFPS) + " -";

        pipe = POPEN(cmd.c_str(), "rb");
        return pipe != nullptr;
    }

    bool VideoPlayer::loadVideo(const std::string& videoPath, int w, int h, int targetFPS) {
        close();

        currentVideoPath = videoPath;
        width = w;
        height = h;
        baseFPS = targetFPS;
        currentTime = 0.0;
        playbackSpeed = 1.0;

        frameBuffer = new uint32_t[width * height];
        duration = getVideoDuration(videoPath);

        // 1. Start Video Pipe
        if (!openPipe(0.0)) return false;

        // 2. Start Audio
        initAudio(videoPath);

        isPlaying = true;
        return true;
    }

    void VideoPlayer::seek(double time) {
        if (time < 0) time = 0;
        if (time > duration) time = duration;

        currentTime = time;

        // Sync Audio
        if (audioInitialized) {
            // ma_sound_seek_to_pcm_frame expects frames, so we convert time -> frames
            // Assuming 44100 or 48000 is handled internally by miniaudio's time conversion if available,
            // but seeking by PCM frame is the standard API.
            // Actually, there isn't a direct "seek to seconds" in the high-level API easily without sample rate.
            // BUT, we can just restart the audio engine at the specific point if needed, or use:
            // ma_uint64 pcmFrame = (ma_uint64)(time * 48000); // Approximation, unsafe.

            // Safer Miniaudio seek:
            // Since we initialized from file, the engine knows the sample rate.
            ma_uint32 sampleRate;
            ma_sound_get_data_format(audioSound, NULL, NULL, &sampleRate, NULL, 0);
            ma_sound_seek_to_pcm_frame(audioSound, (ma_uint64)(time * sampleRate));
        }

        // Sync Video
        openPipe(currentTime);
    }

    void VideoPlayer::setSpeed(double speed) {
        if (speed >= 0.1 && speed <= 10.0) {
            playbackSpeed = speed;
            // Miniaudio supports pitch shifting which changes speed
            // Note: This changes pitch too unless you use a more complex resampler, 
            // but for a simple player, high pitch on 2x speed is standard/expected behavior.
            // ma_sound_set_pitch(audioSound, (float)speed); 
            // Actually, set_pitch does change speed. 
            // Ideally we want time stretching, but that's complex DSP. 
            // Simple pitch shift is 1 line:
            if (audioInitialized) ma_sound_set_pitch(audioSound, (float)speed);
        }
    }

    bool VideoPlayer::update(float deltaTime) {
        if (!isPlaying || !pipe) return false;

        // --- SYNC LOGIC ---
        // If we have audio, use it as the master clock.
        // If not, fall back to deltaTime accumulation (silent video).

        double actualTime = 0.0;

        if (audioInitialized && ma_sound_is_playing(audioSound)) {
            // Get time from audio engine
            // ma_sound_get_time_in_milliseconds returns time adjusted for pitch/speed
            // but for sync we usually want the cursor position in the file.
            // ma_sound_get_cursor_in_pcm_frames / sampleRate = seconds

            ma_uint32 sampleRate;
            ma_sound_get_data_format(audioSound, NULL, NULL, &sampleRate, NULL, 0);
            ma_uint64 cursor;
            ma_sound_get_cursor_in_pcm_frames(audioSound, &cursor);

            if (sampleRate > 0) {
                actualTime = (double)cursor / (double)sampleRate;
            }
            else {
                actualTime = currentTime + deltaTime; // Fallback
            }
        }
        else {
            // No audio track or failed load -> use manual timer
            actualTime = currentTime + (deltaTime * playbackSpeed);
        }

        // Sync check:
        // Video should display the frame corresponding to 'actualTime'.
        // If our internal 'currentTime' (which tracks the last frame read from pipe) is behind 'actualTime',
        // we need to read frames until we catch up.

        // Threshold: Don't read if we are ahead (wait for audio)
        // Only read if actualTime > currentTime

        // Safeguard: If audio loops or seeks unexpectedly, snap video time.
        if (std::abs(actualTime - currentTime) > 2.0) {
            // Seek detected or drift too large, snap video pipe
            seek(actualTime);
        }

        // Read frames loop
        // We might need to skip frames if video is lagging behind audio (e.g. slow decoding)
        int framesRead = 0;
        double frameDuration = 1.0 / baseFPS;

        while (currentTime < actualTime) {
            size_t pixelsToRead = width * height;
            size_t itemsRead = fread(frameBuffer, sizeof(uint32_t), pixelsToRead, pipe);

            if (itemsRead < pixelsToRead) {
                close(); // End of video
                return false;
            }

            currentTime += frameDuration;
            framesRead++;

            // If we are insanely behind (e.g. > 5 frames), just skipping reading might be better?
            // No, pipe is sequential. We MUST read to advance. 
            // FFmpeg pipe is the bottleneck.
            // Limit loop to avoid freezing main thread if sync is wildly off
            if (framesRead > 5) {
                // We are lagging. Break and render what we have, catch up next tick.
                break;
            }
        }

        // If audio isn't present, update currentTime for the next tick manually
        if (!audioInitialized) {
            currentTime = actualTime;
        }

        return true;
    }

    void VideoPlayer::draw(Window& window, int x, int y) {
        if (frameBuffer && isPlaying) {
            window.drawBuffer(x, y, width, height, frameBuffer);
        }
    }

    bool VideoPlayer::drawControls(Window& window, int x, int y, int w, int h) {
        if (!isPlaying) return false;

        bool interacted = false;
        int barHeight = 50;
        int barY = y + h - barHeight;

        // UI Code same as before...
        window.fillRect(x, barY, w, barHeight, Color(20, 20, 20));
        window.drawRect(x, barY, w, barHeight, Color(100, 100, 100));

        int trackMarginX = 20;
        int trackY = barY + 20;
        int trackW = w - (trackMarginX * 2);
        int trackH = 4;

        window.fillRect(x + trackMarginX, trackY, trackW, trackH, Color(80, 80, 80));

        float pct = 0.0f;
        if (duration > 0) pct = (float)(currentTime / duration);
        if (pct > 1.0f) pct = 1.0f;

        int fillW = (int)(trackW * pct);
        window.fillRect(x + trackMarginX, trackY, fillW, trackH, Color(220, 50, 50));

        int knobX = x + trackMarginX + fillW;
        window.fillCircle(knobX, trackY + trackH / 2 - 1, 6, Color(255, 255, 255));

        std::string timeStr = std::to_string((int)currentTime) + "s / " + std::to_string((int)duration) + "s";
        std::string speedStr = "Speed: " + std::to_string(playbackSpeed).substr(0, 3) + "x  [<]  [>]";

        window.drawText(x + trackMarginX, barY + 30, timeStr, Color::White());
        window.drawText(x + w - 180, barY + 30, speedStr, Color::White());

        if (window.isMouseLeftDown()) {
            Vec2 mouse = window.getMousePos();
            if (mouse.x >= x + trackMarginX && mouse.x <= x + trackMarginX + trackW &&
                mouse.y >= barY && mouse.y <= barY + 30) {
                float clickPct = (float)(mouse.x - (x + trackMarginX)) / trackW;
                double newTime = clickPct * duration;
                seek(newTime);
                interacted = true;
            }
            if (window.isButtonClicked(x + w - 80, barY + 30, 25, 15)) {
                setSpeed(playbackSpeed - 0.25);
                interacted = true;
            }
            if (window.isButtonClicked(x + w - 35, barY + 30, 25, 15)) {
                setSpeed(playbackSpeed + 0.25);
                interacted = true;
            }
        }
        return interacted;
    }

    void VideoPlayer::close() {
        isPlaying = false;
        cleanupAudio(); // Stop audio
        if (pipe) {
            PCLOSE(pipe);
            pipe = nullptr;
        }
        if (frameBuffer) {
            delete[] frameBuffer;
            frameBuffer = nullptr;
        }
    }
}