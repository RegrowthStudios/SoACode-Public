#pragma once
#include "stdafx.h"

#include <chrono>
#include <vector>

class PreciseTimer {
public:
    PreciseTimer() : _timerRunning(false) {};
    void start();
    ui32 stop();

    bool isRunning() const { return _timerRunning; }
private:
    bool _timerRunning;
    std::chrono::time_point<std::chrono::high_resolution_clock> _start;
};

class MultiplePreciseTimer {
public:
    struct Interval {
        Interval(std::string Tag) : tag(Tag) {};
        std::string tag;
        ui32 time;
    };

    void start(std::string tag);
    void stop();
    //Prints all timings
    void end();
private:
    PreciseTimer _timer;
    std::vector<Interval> intervals;
};