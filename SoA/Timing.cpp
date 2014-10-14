#include "stdafx.h"
#include <SDL\SDL.h>
#include "Timing.h"

typedef std::chrono::milliseconds ms;

void PreciseTimer::start() {
    _timerRunning = true;
    _start = std::chrono::high_resolution_clock::now();
}

//Returns time in ms
float PreciseTimer::stop() {
    _timerRunning = false;
    std::chrono::duration<float> duration = std::chrono::high_resolution_clock::now() - _start;
    return duration.count() * 1000.0f; //return ms
}

void MultiplePreciseTimer::start(std::string tag) {
    if (_timer.isRunning()) stop();
    if (_samples == 0) intervals.push_back(Interval(tag));

    _timer.start();
     
}

void MultiplePreciseTimer::stop() {
    intervals[_index++].time += _timer.stop();
}

//Prints all timings
void MultiplePreciseTimer::end(bool print) {
    if (_timer.isRunning()) _timer.stop();
    if (intervals.empty()) return;
    if (_samples == _desiredSamples) {
        if (print) {
            printf("TIMINGS: \n");
            for (int i = 0; i < intervals.size(); i++) {
                printf("  %-20s: %12f ms\n", intervals[i].tag.c_str(), intervals[i].time / _samples);
            }
            printf("\n");
        }
        intervals.clear();
        _samples = 0;
        _index = 0;
    } else {
        _index = 0;
        _samples++;
    }
}

FpsLimiter::FpsLimiter() {
}
void FpsLimiter::init(float maxFPS) {
    setMaxFPS(maxFPS);
}

void FpsLimiter::setMaxFPS(float maxFPS) {
    _maxFPS = maxFPS;
}

void FpsLimiter::begin() {
    _startTicks = SDL_GetTicks();
}

float FpsLimiter::end() {
    calculateFPS();

    float frameTicks = SDL_GetTicks() - _startTicks;
    //Limit the FPS to the max FPS
    if (1000.0f / _maxFPS > frameTicks) {
        SDL_Delay((Uint32)(1000.0f / _maxFPS - frameTicks));
    }

    return _fps;
}

void FpsLimiter::calculateFPS() {
    //The number of frames to average
    static const int NUM_SAMPLES = 10;
    //Stores all the frametimes for each frame that we will average
    static float frameTimes[NUM_SAMPLES];
    //The current frame we are on
    static int currentFrame = 0;
    //the ticks of the previous frame
    static float prevTicks = SDL_GetTicks();

    //Ticks for the current frame
    float currentTicks = SDL_GetTicks();

    //Calculate the number of ticks (ms) for this frame
    _frameTime = currentTicks - prevTicks;
    frameTimes[currentFrame % NUM_SAMPLES] = _frameTime;

    //current ticks is now previous ticks
    prevTicks = currentTicks;

    //The number of frames to average
    int count;

    currentFrame++;
    if (currentFrame < NUM_SAMPLES) {
        count = currentFrame;
    } else {
        count = NUM_SAMPLES;
    }

    //Average all the frame times
    float frameTimeAverage = 0;
    for (int i = 0; i < count; i++) {
        frameTimeAverage += frameTimes[i];
    }
    frameTimeAverage /= count;

    //Calculate FPS
    if (frameTimeAverage > 0) {
        _fps = 1000.0f / frameTimeAverage;
    } else {
        _fps = 60.0f;
    }
}