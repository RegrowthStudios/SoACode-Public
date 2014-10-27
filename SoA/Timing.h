#pragma once
#include "stdafx.h"

#include <chrono>
#include <vector>

class PreciseTimer {
public:
    PreciseTimer() : _timerRunning(false) {};
    void start();
    float stop();

    bool isRunning() const { return _timerRunning; }
private:
    bool _timerRunning;
    std::chrono::time_point<std::chrono::high_resolution_clock> _start;
};

class MultiplePreciseTimer {
public:
    

    MultiplePreciseTimer() : _samples(0), _desiredSamples(1), _index(0) {};

    void start(std::string tag);
    void stop();
    //Prints all timings
    void end(bool print);
    void setDesiredSamples(int desiredSamples) { _desiredSamples = desiredSamples; }

private:
    struct Interval {
        Interval(std::string Tag) : tag(Tag), time(0.0f) {};
        std::string tag;
        float time;

    };
    int _samples;
    int _desiredSamples;
    int _index;
    PreciseTimer _timer;
    std::vector<Interval> intervals;
};

/// Calculates FPS 
class FpsCounter {
public:
    FpsCounter();
    
    /// Begins a frame timing for fps calculation
    void beginFrame();

    /// ends the frame
    /// @return The current FPS as a float
    float endFrame();

    /// Returns the current fps as last recorded by an endFrame
    float getCurrentFps() const { return _fps; }
protected:
    // Calculates the current FPS
    void calculateFPS();

    // Variables
    float _fps;
    float _frameTime;
    unsigned int _startTicks;
};

///Calculates FPS and also limits FPS
class FpsLimiter : public FpsCounter {
public:
    FpsLimiter();

    // Initializes the FPS limiter. For now, this is
    // analogous to setMaxFPS
    void init(float maxFPS);

    // Sets the desired max FPS
    void setMaxFPS(float maxFPS);

    // end() will return the current FPS as a float and limit fps
    float endFrame();
private:
    float _maxFPS;
};