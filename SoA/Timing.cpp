#include "stdafx.h"
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