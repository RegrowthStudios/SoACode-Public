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
    void end();
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