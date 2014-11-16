#include "stdafx.h"
#include <SDL/SDL.h>
#include "Timing.h"

typedef std::chrono::milliseconds ms;

const float MS_PER_SECOND = 1000.0f;

void PreciseTimer::start() {
    _timerRunning = true;
    _start = std::chrono::high_resolution_clock::now();
}

//Returns time in ms
float PreciseTimer::stop() {
    _timerRunning = false;
    std::chrono::duration<float> duration = std::chrono::high_resolution_clock::now() - _start;
    return duration.count() * MS_PER_SECOND; //return ms
}

void MultiplePreciseTimer::start(std::string tag) {
    if (_index >= intervals.size()) intervals.push_back(Interval(tag));
    if (_timer.isRunning()) stop();
    
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

FpsCounter::FpsCounter() : _fps(0) {
    // Empty
}
void FpsCounter::beginFrame() {
    _startTicks = SDL_GetTicks();
}

float FpsCounter::endFrame() {
    calculateFPS();
    return _fps;
}

void FpsCounter::calculateFPS() {
    
    #define DECAY 0.9

    //Calculate the number of ticks (ms) for this frame
    float timeThisFrame = SDL_GetTicks() - _startTicks;
    // Use a simple moving average to decay the FPS
    _frameTime = _frameTime * DECAY + timeThisFrame * (1.0f - DECAY);
  
    //Calculate FPS
    if (_frameTime > 0.0f) {
        _fps = MS_PER_SECOND / _frameTime;
    } else {
        _fps = 60.0f;
    }
}

FpsLimiter::FpsLimiter() {
    // Empty
}
void FpsLimiter::init(float maxFPS) {
    setMaxFPS(maxFPS);
}

void FpsLimiter::setMaxFPS(float maxFPS) {
    _maxFPS = maxFPS;
}

float FpsLimiter::endFrame() {
    calculateFPS();

    float frameTicks = SDL_GetTicks() - _startTicks;
    //Limit the FPS to the max FPS
    if (MS_PER_SECOND / _maxFPS > frameTicks) {
        SDL_Delay((Uint32)(MS_PER_SECOND / _maxFPS - frameTicks));
    }

    return _fps;
}