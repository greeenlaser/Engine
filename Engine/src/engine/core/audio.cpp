//Copyright(C) 2025 Lost Empire Entertainment
//This program comes with ABSOLUTELY NO WARRANTY.
//This is free software, and you are welcome to redistribute it under certain conditions.
//Read LICENSE.md for more information.

#include <unordered_map>
#include <memory>
#include <filesystem>
#include <iostream>

//external
#define MINIAUDIO_IMPLEMENTATION
#include "miniaudio.h"

//engine
#include "audio.hpp"
#include "console.hpp"
#include "core.hpp"

using std::unordered_map;
using std::unique_ptr;
using std::make_unique;
using std::move;
using std::filesystem::exists;
using std::filesystem::path;
using std::cout;
using std::to_string;

using Core::ConsoleManager;
using Caller = Core::ConsoleManager::Caller;
using Type = Core::ConsoleManager::Type;
using Core::Engine;

namespace Core
{
    static ma_engine engine;
    unordered_map<string, unique_ptr<ma_sound>> soundMap;

    bool Audio::Initialize()
    {
        ma_result result = ma_engine_init(NULL, &engine);
        if (result != MA_SUCCESS)
        {
            ConsoleManager::WriteConsoleMessage(
                Caller::INITIALIZE,
                Type::EXCEPTION,
                "Error: Failed to initialize miniaudio!\n");
            return false;
        }

        ConsoleManager::WriteConsoleMessage(
            Caller::INITIALIZE,
            Type::DEBUG,
            "Miniaudio initialized successfully!\n");

        return true;
    }

    bool Audio::Import(const string& name)
    {
        string fullPath = (path(Engine::projectPath) / "audio" / name).string();

        if (!exists(fullPath))
        {
            ConsoleManager::WriteConsoleMessage(
                Caller::FILE,
                Type::EXCEPTION,
                "Error: Cannot import audio file because it is not found: " + fullPath + "\n");
            return false;
        }

        if (soundMap.find(name) != soundMap.end())
        {
            ConsoleManager::WriteConsoleMessage(
                Caller::FILE,
                Type::INFO,
                "Error: Audio file already imported: " + name + "\n");
            return false;
        }

        auto sound = make_unique<ma_sound>();
        ma_result result = ma_sound_init_from_file(&engine, fullPath.c_str(), 0, NULL, NULL, sound.get());
        if (result != MA_SUCCESS)
        {
            ConsoleManager::WriteConsoleMessage(
                Caller::FILE,
                Type::EXCEPTION,
                "Error: Failed to import audio file: " + name + "\n");
            return false;
        }

        soundMap[name] = move(sound);

        ConsoleManager::WriteConsoleMessage(
            Caller::FILE,
            Type::INFO,
            "Successfully imported audio file: " + name + "\n");

        Play(name);
        return true;
    }

    bool Audio::Play(const string& name)
    {
        string fullPath = (path(Engine::projectPath) / "audio" / name).string();

        if (!IsImported(name))
        {
            Import(name);
        }

        auto it = soundMap.find(name);
        if (it == soundMap.end())
        {
            ConsoleManager::WriteConsoleMessage(
                Caller::FILE,
                Type::EXCEPTION,
                "Error: Audio file not found in sound map: " + name + "\n");
            return false;
        }

        //reset audio to the beginning
        ma_result seekResult = ma_sound_seek_to_pcm_frame(it->second.get(), 0);
        if (seekResult != MA_SUCCESS)
        {
            ConsoleManager::WriteConsoleMessage(
                Caller::FILE,
                Type::EXCEPTION,
                "Error: Failed to reset audio playback position: " + name + "\n");
            return false;
        }

        ma_result result = ma_sound_start(it->second.get());
        if (result != MA_SUCCESS)
        {
            ConsoleManager::WriteConsoleMessage(
                Caller::FILE,
                Type::EXCEPTION,
                "Error: Failed to play audio file: " + name + "\n");
            return false;
        }

        ConsoleManager::WriteConsoleMessage(
            Caller::FILE,
            Type::DEBUG,
            "Playing audio file from the beginning: " + name + "\n");

        return true;
    }
    bool Audio::Stop(const string& name)
    {
        string fullPath = (path(Engine::projectPath) / "audio" / name).string();

        if (!IsImported(name))
        {
            ConsoleManager::WriteConsoleMessage(
                Caller::FILE,
                Type::EXCEPTION,
                "Error: Cannot stop audio file because it has not been imported: " + fullPath + "\n");
            return false;
        }

        auto it = soundMap.find(name);
        if (it == soundMap.end())
        {
            ConsoleManager::WriteConsoleMessage(
                Caller::FILE,
                Type::EXCEPTION,
                "Error: Audio file not found in sound map: " + name + "\n");
            return false;
        }

        ma_result result = ma_sound_stop(it->second.get());
        if (result != MA_SUCCESS)
        {
            ConsoleManager::WriteConsoleMessage(
                Caller::FILE,
                Type::EXCEPTION,
                "Error: Failed to stop audio file: " + name + "\n");
            return false;
        }

        //reset the audio position to the start, so the next play starts fresh
        ma_result seekResult = ma_sound_seek_to_pcm_frame(it->second.get(), 0);
        if (seekResult != MA_SUCCESS)
        {
            ConsoleManager::WriteConsoleMessage(
                Caller::FILE,
                Type::EXCEPTION,
                "Error: Failed to reset audio playback position after stopping: " + name + "\n");
            return false;
        }

        ConsoleManager::WriteConsoleMessage(
            Caller::FILE,
            Type::DEBUG,
            "Stopped audio file and reset position: " + name + "\n");

        return true;
    }

    bool Audio::Pause(const string& name)
    {
        string fullPath = (path(Engine::projectPath) / "audio" / name).string();

        if (!IsImported(name))
        {
            ConsoleManager::WriteConsoleMessage(
                Caller::FILE,
                Type::EXCEPTION,
                "Error: Cannot pause audio file because it has not been imported: " + fullPath + "\n");
            return false;
        }

        auto it = soundMap.find(name);
        if (it == soundMap.end())
        {
            ConsoleManager::WriteConsoleMessage(
                Caller::FILE,
                Type::EXCEPTION,
                "Error: Audio file not found in sound map: " + name + "\n");
            return false;
        }

        ma_result result = ma_sound_stop(it->second.get());
        if (result != MA_SUCCESS)
        {
            ConsoleManager::WriteConsoleMessage(
                Caller::FILE,
                Type::EXCEPTION,
                "Error: Failed to pause audio file: " + name + "\n");
            return false;
        }

        ConsoleManager::WriteConsoleMessage(
            Caller::FILE,
            Type::DEBUG,
            "Paused audio file: " + name + "\n");

        return true;
    }
    bool Audio::Continue(const string& name)
    {
        string fullPath = (path(Engine::projectPath) / "audio" / name).string();

        if (!IsImported(name))
        {
            ConsoleManager::WriteConsoleMessage(
                Caller::FILE,
                Type::EXCEPTION,
                "Error: Cannot continue playing audio file because it has not been imported: " + fullPath + "\n");
            return false;
        }

        auto it = soundMap.find(name);

        ma_result result = ma_sound_start(it->second.get());
        if (result != MA_SUCCESS)
        {
            ConsoleManager::WriteConsoleMessage(
                Caller::FILE,
                Type::EXCEPTION,
                "Error: Failed to continue playing audio file: " + name + "\n");
            return false;
        }

        ConsoleManager::WriteConsoleMessage(
            Caller::FILE,
            Type::DEBUG,
            "Continuing playing audio file: " + name + "\n");

        return true;
    }

    void Audio::SetVolume(const string& name, float volume)
    {
        auto it = soundMap.find(name);
        if (it == soundMap.end())
        {
            ConsoleManager::WriteConsoleMessage(
                Caller::FILE,
                Type::EXCEPTION,
                "Error: Cannot set volume because audio file has not been imported: " + name + "\n");
            return;
        }

        //convert the volume from 0.0f - 100.0f range to 0.0f - 1.0f range
        float scaledVolume = volume / 100.0f;

        //set the volume using Miniaudio API
        ma_sound_set_volume(it->second.get(), scaledVolume);

        ConsoleManager::WriteConsoleMessage(
            Caller::FILE,
            Type::DEBUG,
            "Volume set to " + to_string(volume) + "% for audio file: " + name + "\n");
    }

    bool Audio::Set3DState(const string& name, bool state)
    {
        auto it = soundMap.find(name);
        if (it == soundMap.end())
        {
            ConsoleManager::WriteConsoleMessage(
                Caller::FILE,
                Type::EXCEPTION,
                "Error: Cannot set 2D state because audio file has not been imported: " + name + "\n");
            return false;
        }

        if (!state)
        {
            //enable 2D sound (absolute positioning)
            ma_sound_set_positioning(it->second.get(), ma_positioning_absolute);

            UpdatePlayerPosition(name, vec3(0));
        }
        else
        {
            //enable 3D sound (relative positioning)
            ma_sound_set_positioning(it->second.get(), ma_positioning_relative);
        }

        ConsoleManager::WriteConsoleMessage(
            Caller::FILE,
            Type::INFO,
            "2D state set to " + string(state ? "enabled" : "disabled") + " for audio file: " + name + "\n");

        return true;
    }

    void Audio::UpdateListenerPosition(const vec3& pos)
    {
        ma_vec3f tempPos = ma_engine_listener_get_position(&engine, 0);
        vec3 currPos(tempPos.x, tempPos.y, tempPos.z);
        if (pos != currPos)
        {
            ma_engine_listener_set_position(&engine, 0, pos.x, pos.y, pos.z);
        }
    }
    bool Audio::UpdatePlayerPosition(const string& name, const vec3& pos)
    {
        auto it = soundMap.find(name);
        if (it == soundMap.end())
        {
            ConsoleManager::WriteConsoleMessage(
                Caller::FILE,
                Type::EXCEPTION,
                "Error: Cannot update position because audio file has not been imported: " + name + "\n");
            return false;
        }

        ma_sound* sound = it->second.get();

        //update the position of the specific audio source
        ma_vec3f tempPos = ma_sound_get_position(sound);
        vec3 currPos(tempPos.x, tempPos.y, tempPos.z);
        if (pos != currPos)
        {
            ma_sound_set_position(sound, pos.x, pos.y, pos.z);
        }

        return true;
    }

    bool Audio::Delete(const string& name)
    {
        string fullPath = (path(Engine::projectPath) / "audio" / name).string();

        if (!IsImported(name))
        {
            ConsoleManager::WriteConsoleMessage(
                Caller::FILE,
                Type::EXCEPTION,
                "Error: Cannot delete audio file because it has not been imported: " + name + "\n");
            return false;
        }

        auto it = soundMap.find(name);

        ma_sound_uninit(it->second.get());
        soundMap.erase(it);

        ConsoleManager::WriteConsoleMessage(
            Caller::FILE,
            Type::DEBUG,
            "Successfully deleted audio file: " + name + "\n");

        return true;
    }

    bool Audio::IsImported(const string& name)
    {
        string fullPath = (path(Engine::projectPath) / "audio" / name).string();

        auto it = soundMap.find(name);
        return it != soundMap.end();
    }

    void Audio::Shutdown()
    {
        ma_engine_uninit(&engine);
    }
}