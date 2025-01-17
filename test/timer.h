//
// Created by user on 25-1-16.
//

#ifndef TIMER_H
#define TIMER_H

#include <chrono>
class timer
{
private:
    std::chrono::time_point<std::chrono::system_clock> start;
    std::chrono::time_point<std::chrono::system_clock> end;

public:
    timer() = default;
    void start_timing()
    {
        start = std::chrono::high_resolution_clock::now();
    }
    void end_timing()
    {
        end = std::chrono::high_resolution_clock::now();
    }
    [[nodiscard]] long cost_time_for_us() const
    {
        const auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
        return duration.count();
    }
};

#endif //TIMER_H
