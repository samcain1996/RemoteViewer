#include <chrono>
#include <functional>
#include <thread>
#include <atomic>
#include <mutex>

using std::function;
using std::chrono::high_resolution_clock;
using std::thread;
using std::atomic;
using std::chrono::milliseconds;

class Timer {

private:
    int interval;
    atomic<bool> active;
    function<void()> onTick;
    high_resolution_clock::time_point prevTicks;
    thread functionThread;

public:
    Timer(int interval, function<void()> onTick);

    void start();
    void stop();
    bool isRunning() const;

    long long timeSinceLastInterval() const;

private:
    void timerloop();

};