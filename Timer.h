#pragma once

#include <chrono>

class Time
{
private:
    Time() { StartTime = std::chrono::steady_clock::now(); }

    std::chrono::steady_clock::time_point StartTime;
    std::chrono::duration<double> DeltaTime;

public:
    static float TimeScale;
    static double deltaTime;

    Time(const Time &) = delete;
    void operator=(const Time &) = delete;

    static Time &GetInstance();

    static void Tick();
    static void Reset();
};

double Time::deltaTime = 0.0;
float Time::TimeScale = 1.0f;

Time &Time::GetInstance()
{
    static Time instance;
    return instance;
}

void Time::Tick()
{
    auto &instance = GetInstance();
    instance.DeltaTime = std::chrono::steady_clock::now() - instance.StartTime;
    deltaTime = instance.DeltaTime.count() * TimeScale;
}

void Time::Reset()
{
    auto &instance = GetInstance();
    instance.StartTime = std::chrono::steady_clock::now();
}