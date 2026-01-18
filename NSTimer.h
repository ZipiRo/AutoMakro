#pragma once

#include <chrono>

class Time
{
private:
    std::chrono::steady_clock::time_point StartTime;
    std::chrono::duration<double> DeltaTime;

public:
    Time() { 
        StartTime = std::chrono::steady_clock::now(); 
        TimeScale = 1.0f;
        deltaTime = 0.0f;
    }
    
    float TimeScale;
    double deltaTime;

    void Tick();
    void Reset();
};

void Time::Tick()
{
    DeltaTime = std::chrono::steady_clock::now() - StartTime;
    deltaTime = DeltaTime.count() * TimeScale;
}

void Time::Reset()
{
    StartTime = std::chrono::steady_clock::now();
}