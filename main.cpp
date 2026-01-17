/// THIS IS A MAKRO MAKER FOR WINDOWS
/// VERSION 1.0 
/// MADE BY ZIPIRO

#include <iostream>
#include <fstream>
#include <vector>
#include <thread>
#include <chrono>
#include <atomic>
#include <limits>
#include <string>

namespace W {
    #include <windows.h>
}

#include "Timer.h"

float FPS = 60;
std::atomic<bool> running;
std::thread INPUT_THREAD;
W::HWND CONSOLE_WINDOW;

std::string GetKeyName(int key)
{
    W::UINT scan = W::MapVirtualKey(key, MAPVK_VK_TO_VSC);

    bool isExtended =
        key == VK_RIGHT || key == VK_LEFT ||
        key == VK_UP    || key == VK_DOWN ||
        key == VK_INSERT || key == VK_DELETE ||
        key == VK_HOME || key == VK_END ||
        key == VK_PRIOR || key == VK_NEXT ||
        key == VK_RCONTROL || key == VK_RMENU;

    W::LONG lParam = (scan << 16);
    if (isExtended)
        lParam |= (1 << 24);

    char name[128] = {};
    W::GetKeyNameTextA(lParam, name, sizeof(name));

    return std::string(name);
}

bool IsFocused()
{
    return W::GetForegroundWindow() == CONSOLE_WINDOW;
}

struct KEY
{
    int key;
    bool pressed;
    bool wasPressed;
};

KEY keys[256];

void PoolInput()
{
    for(int key = 0; key < 256; key++)
    {
        keys[key].wasPressed = keys[key].pressed;
        keys[key].pressed = (W::GetAsyncKeyState(key) & 0x8000) != 0;
    }
}

bool IsKeyDown(int key)
{
    return keys[key].pressed && !keys[key].wasPressed;
}

bool IsKey(int key)
{
    return keys[key].pressed;
}

void InputThread()
{
    while(running)
    {
        if(!IsFocused()) continue;
        PoolInput();
        std::this_thread::sleep_for(std::chrono::milliseconds(5));
    }
}

void PressKey(int key)
{
    W::INPUT input[2] = {};

    W::WORD scan = W::MapVirtualKey(key, MAPVK_VK_TO_VSC);

    bool isEXT = 
        key == VK_RIGHT || key == VK_LEFT ||
        key == VK_UP    || key == VK_DOWN ||
        key == VK_INSERT || key == VK_DELETE ||
        key == VK_HOME || key == VK_END ||
        key == VK_PRIOR || key == VK_NEXT;

    input[0].type = INPUT_KEYBOARD;
    input[0].ki.wVk = 0;
    input[0].ki.wScan = scan;
    input[0].ki.dwFlags = KEYEVENTF_SCANCODE; 
    if(isEXT)
        input[0].ki.dwFlags |= KEYEVENTF_EXTENDEDKEY;
    
    input[1].type = INPUT_KEYBOARD;
    input[1].ki.wVk = 0;
    input[1].ki.wScan = scan;
    input[1].ki.dwFlags = KEYEVENTF_SCANCODE | KEYEVENTF_KEYUP;
    if(isEXT)
        input[1].ki.dwFlags |= KEYEVENTF_EXTENDEDKEY;

    W::SendInput(2, input, sizeof(W::INPUT));
}

std::vector<int> recorded_keys;

bool recording_keys = false;

void SaveRecording()
{
    std::ofstream file(".record", std::ios::binary);
    
    uint8_t count = (uint8_t)recorded_keys.size();
    file.write((char*)&count, sizeof(count));

    for(auto key : recorded_keys)
        file.write((char*)&key, sizeof(char));

    file.flush();
    file.close();
}

void RecordKeys()
{
    if(IsKey(VK_ESCAPE))
    {
        recording_keys = false;
        SaveRecording();
        std::cout << "STOP RECORDING\n";
        return;
    }

    for(int key = 0; key < 256; key++)
    {
        if(IsKeyDown(key)){
            recorded_keys.emplace_back(key);
            std::cout << GetKeyName(key) << '\n';
        }
    }
}

bool playing_recording = false;
bool playing_keys = false;
std::vector<int> play_keys;
int playing_key_index = 0;

float delay_betwen_keys = 0;
float delay_timer = 0;

void FetchRecording()
{
    play_keys.clear();
    std::ifstream file(".record", std::ios::binary);
    
    int count = 0;
    file.read((char*)&count, sizeof(char));

    play_keys.resize(count);
    for (int i = 0; i < count; i++)
    {
        uint8_t key;
        file.read((char*)&key, sizeof(key));
        play_keys[i] = key;
    }

    file.close();

    for(auto key : play_keys)
        std::cout << GetKeyName(key) << " => ";
    std::cout << '\n'; 
}

void PlayKeys()
{
    int key = play_keys[playing_key_index];
    playing_key_index++;

    if(playing_key_index > play_keys.size() - 1)
        playing_key_index = 0;

    PressKey(key);
}

void Play()
{
    delay_timer += Time::deltaTime; 
    if(delay_timer >= delay_betwen_keys)
    {
        delay_timer = 0;
        PlayKeys();
    }
}

int main()
{
    running = true;
    Time::GetInstance();
    CONSOLE_WINDOW = W::GetConsoleWindow();
    
    INPUT_THREAD = std::thread(&InputThread);

    std::cout << "------------AutoMakro------------\n";
    std::cout << "R - RECORD KEYBOARD\n";
    std::cout << "F12 - PLAY_RECORD\n";
    std::cout << "ESC - QUIT\n";
    std::cout << "---------------------------------\n";

    while (running)
    {
        Time::Tick();
        
        if(Time::deltaTime >= 1.0f / FPS)
        {
            Time::Reset();       

            if(recording_keys)
                RecordKeys(); 
            else if(playing_recording)
            {
                if((W::GetAsyncKeyState(VK_ESCAPE) & 0x8000) != 0) /// STOP EVEN IF NOT FOCUSED
                {
                    playing_keys = false;
                    playing_recording = false;
                    playing_key_index = 0;
                    delay_timer = 0;

                    std::cout << "STOP PLAYING\n";
                }
            }
            else
            {
                if(IsKeyDown(int('R')))
                {
                    recorded_keys.clear();
                    recording_keys = true;

                    std::cout << "RECORDING...\n"; 
                }

                if((W::GetAsyncKeyState(VK_F12) & 0x8000) != 0) /// PLAY ON F12 EVEN IF NOT FOCUSED
                {
                    playing_keys = true;
                    playing_recording = true;
                    FetchRecording();

                    std::cout << "\nPLAYING...\n";
                }

                if(IsKeyDown(VK_ESCAPE))
                    running = false;
            }
        }

        if(playing_keys)
            Play();
    }

    if(INPUT_THREAD.joinable())
        INPUT_THREAD.join();

    return 0;
}