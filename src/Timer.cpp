#include "Timer.h"

Timer::Timer(int interval, function<void()> onTick) : 
    interval(interval), onTick(onTick) { active = false; };

void Timer::start() {
    active = true;
    prevTicks = high_resolution_clock::now();
    functionThread = thread(&Timer::timerloop, this);
}

void Timer::stop() {
    active = false;
    if (functionThread.joinable()) { functionThread.join(); }
}

long long Timer::timeSinceLastInterval() const {
    return duration_cast<milliseconds>(high_resolution_clock::now() - prevTicks).count();
}

bool Timer::isRunning() const { return active; }

void Timer::timerloop() {
    while (active) {

        high_resolution_clock::time_point curTicks = high_resolution_clock::now();
        int delta = duration_cast<milliseconds>(curTicks - prevTicks).count();

        if (delta > interval) {
            onTick();
            prevTicks = curTicks;
        }
    }
}