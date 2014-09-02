#include "stdafx.h"
#include "Timing.h"


void PreciseTimer::start() {
    _timerRunning = true;
    _start = std::chrono::high_resolution_clock::now();
}

ui32 PreciseTimer::stop() {
    _timerRunning = false;
    return std::chrono::high_resolution_clock::now().time_since_epoch - _start.time_since_epoch;
}

void MultiplePreciseTimer::start(std::string tag) {
    intervals.push_back(Interval(tag));
    _timer.start();
}

void MultiplePreciseTimer::stop() {
    intervals.back().time = _timer.stop();
}

//Prints all timings
void MultiplePreciseTimer::end() {
    if (intervals.empty()) return;

    printf("TIMINGS: \n");
    for (int i = 0; i < intervals.size(); i++) {
        printf("  %10s: %10u\n", intervals[i].tag.c_str(), intervals[i].time);
    }
    printf("\n");
    intervals.clear();
}