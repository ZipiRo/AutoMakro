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

using W::DWORD;

#include "Timer.h"

float FPS = 60;
std::atomic<bool> running;
std::thread INPUT_THREAD;
std::thread PLAY_THREAD;
W::HWND CONSOLE_WINDOW;
W::HANDLE CONSOLE_INPUT_HANDLE;

Time main_Time;
Time play_Time;

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
    bool focused;
};

KEY keys[256];

void PoolInput()
{
    for(int key = 0; key < 256; key++)
    {
        keys[key].wasPressed = keys[key].pressed;
        keys[key].pressed = (W::GetAsyncKeyState(key) & 0x8000) != 0;
        keys[key].focused = IsFocused();
    }
}

bool IsKeyDown(int key, bool needs_focus = true)
{
    return keys[key].pressed && !keys[key].wasPressed && (!needs_focus || key[keys].focused);
}

bool IsKey(int key, bool needs_focus = true)
{
    return keys[key].pressed && (!needs_focus || key[keys].focused);
}

void InputThread()
{
    while(running)
    {
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

/// RECORDING KEYBOARD MAKRO

std::vector<int> recorded_keys;
bool recording_keys = false;
void SaveRecording()
{
    std::string file_name;

    W::FlushConsoleInputBuffer(CONSOLE_INPUT_HANDLE);
    std::cout << "RECORD NAME: ";
    std::cin >> file_name;
    std::cout << '\n';

    file_name += ".rec";

    std::ofstream file(file_name, std::ios::binary);
    
    uint8_t count = (uint8_t)recorded_keys.size();
    file.write((char*)&count, sizeof(count));

    for(auto key : recorded_keys)
        file.write((char*)&key, sizeof(char));

    file.flush();
    file.close();
    
    std::cout << "SAVED----------------------------\n";
}

void RecordKeys()
{
    if(IsKey(VK_ESCAPE))
    {
        recording_keys = false;
        std::cout << "STOP RECORDING\n";
        std::cout << "---------------------------------\n";
        SaveRecording();
        
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

/// PLAYING RECORDED KEYS

bool playing_recording = false;
std::atomic<bool> playing_keys = false;
std::vector<int> play_keys;
int playing_key_index = 0;

float delay_betwen_keys = 1.0f;
float delay_timer = 0;

float delay_start = 1.0f;
float delay_start_timer = 0;

bool FetchRecording()
{
    std::string file_name;

    W::FlushConsoleInputBuffer(CONSOLE_INPUT_HANDLE);
    std::cout << "RECORD FILE: ";
    std::cin >> file_name;
    std::cout << '\n';

    play_keys.clear();
    std::ifstream file(file_name, std::ios::binary);
    
    if(!file)
        return false;

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

    std::cout << "LOADED---------------------------\n";
    return true;
}

void PlayKeys()
{
    if(delay_timer >= delay_betwen_keys)
    {
        delay_timer = 0;

        int key = play_keys[playing_key_index];
        playing_key_index++;

        if(playing_key_index > play_keys.size() - 1)
            playing_key_index = 0;

        PressKey(key);
    }
}

void PlayThread()
{
    while(running)
    {
        if(!playing_keys)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(5));
            continue;
        }
        
        play_Time.Tick();
        if(play_Time.deltaTime >= 1.0f / 9999)
        {
            play_Time.Reset();

            delay_timer += play_Time.deltaTime;
            PlayKeys();
        }
    }
}

/// MENUS AND STTINGS INTERFACE
void Menu_UI()
{
    std::cout << "------------AutoMakro------------\n";
    std::cout << "| R - RECORD_KEYBOARD\n";
    std::cout << "| L - LOAD_RECORD\n";
    std::cout << "| F12 | F9 - PLAY_RECORD\n";
    std::cout << "| S - SETTINGS\n";
    std::cout << "| DEL - CLEAR SCREEN\n";
    std::cout << "| ESC - QUIT\n";
    std::cout << "---------------------------------\n";
}

void Settings_UI()
{
    std::cout << "-------------Settings------------\n";
    std::cout << "| 1 - DELAY_BETWEN_KEYS( "<< delay_betwen_keys <<"s )\n";
    std::cout << "| 2 - START_DELAY( "<< delay_start <<"s )\n";
    std::cout << "| ESC - MAIN MENU\n";
    std::cout << "---------------------------------\n";
}

bool settings = false;
void Settings()
{
    if(IsKeyDown(VK_ESCAPE))
    {
        system("cls");
        settings = false;
        Menu_UI();
    }

    if(IsKeyDown(int('1')))
    {
        W::FlushConsoleInputBuffer(CONSOLE_INPUT_HANDLE);
        float delay = 0.0f;
        std::cout << "DELAY IN SECONDS (> 0.005): ";
        std::cin >> delay;

        if(delay < 0.005f)
            delay = 0.005f;
            
        delay_betwen_keys = delay;
        system("cls");
        Settings_UI();
    }
    else if(IsKeyDown(int('2')))
    {
        W::FlushConsoleInputBuffer(CONSOLE_INPUT_HANDLE);
        float delay = 0.0f;
        std::cout << "START DELAY IN SECONDS (>= 0): ";
        std::cin >> delay;

        if(delay < 0)
            delay = 0;
            
        delay_start = delay;
        system("cls");
        Settings_UI();
    }
}

int main()
{
    running = true;
    main_Time = Time();
    
    CONSOLE_WINDOW = W::GetConsoleWindow();
    CONSOLE_INPUT_HANDLE = W::GetStdHandle(STD_INPUT_HANDLE);

    INPUT_THREAD = std::thread(&InputThread);
    PLAY_THREAD = std::thread(&PlayThread);

    system("cls");
    Menu_UI();

    while (running)
    {
        main_Time.Tick();
        
        if(main_Time.deltaTime >= 1.0f / FPS)
        {
            main_Time.Reset();       

            if(recording_keys)
            {
                RecordKeys(); 
            }
            else if(playing_recording)
            { 
                if(IsKeyDown(VK_ESCAPE, false))
                {
                    playing_keys = false;
                    playing_recording = false;

                    std::cout << "STOP PLAYING\n";
                    std::cout << "---------------------------------\n";
                }
                
                if(!playing_keys) 
                {
                    delay_start_timer += main_Time.deltaTime;
                    if(delay_start_timer >= delay_start)
                    {
                        playing_keys = true;
                        playing_key_index = 0;
                        delay_timer = 0;
                        delay_start_timer = 0;
                        play_Time = Time();

                        std::cout << "\nPLAYING...\n";
                        std::cout << "ESC TO STOP\n";
                        std::cout << "---------------------------------\n";
                    }
                }    
            }
            else if(settings)
            {
                Settings();
            }
            else
            {
                /// RECORD KEYBOARD MAKRO
                if(IsKeyDown(int('R')))
                {
                    recorded_keys.clear();
                    recording_keys = true;

                    std::cout << "\nRECORDING...\n"; 
                    std::cout << "ESC TO STOP\n";
                    std::cout << "---------------------------------\n";
                }

                /// LOAD KEYBOARD RECORD
                if(IsKeyDown(int('L')))
                {
                    if(!FetchRecording())
                        std::cout << "FILE WAS NOT LOADED!\n";
                }

                if(IsKeyDown(int('S')))
                {
                    system("cls");
                    Settings_UI();
                    settings =  true;
                }

                /// PLAY KEYBOARD MAKRO
                if(IsKeyDown(VK_F12, false) || IsKeyDown(VK_F9, false))
                {
                    if(!play_keys.empty())
                    {
                        playing_recording = true;
                        std::cout << "STARTING IN " << delay_start << "s\n";
                    }
                    else
                    {
                        std::cout << "NO RECORD LOADED!\n";
                    }
                }

                if(IsKeyDown(VK_DELETE))
                {
                    system("cls");
                    Menu_UI();
                }

                if(IsKeyDown(VK_ESCAPE))
                    running = false;
            }
        }
    }

    if(INPUT_THREAD.joinable())
        INPUT_THREAD.join();
    
    if(PLAY_THREAD.joinable())
        PLAY_THREAD.join();

    return 0;
}