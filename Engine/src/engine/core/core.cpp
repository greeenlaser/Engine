//Copyright(C) 2025 Lost Empire Entertainment
//This program comes with ABSOLUTELY NO WARRANTY.
//This is free software, and you are welcome to redistribute it under certain conditions.
//Read LICENSE.md for more information.

#define NOMINMAX
#ifdef _WIN32
#include <Windows.h>
#include <ShlObj.h>
#include <TlHelp32.h>
#include <tchar.h>
#elif __linux__
#include <unistd.h>
#endif
#include <filesystem>
#include <fstream>
#include <cstdlib>
#include <memory>

//external
#include "glfw3.h"
#if DISCORD_MODE
#include "discord.h"
#endif

//engine
#include "core.hpp"
#include "console.hpp"
#include "render.hpp"
#include "timeManager.hpp"
#include "configFile.hpp"
#include "sceneFile.hpp"
#include "fileUtils.hpp"
#include "stringUtils.hpp"
#include "gameobject.hpp"
#include "audio.hpp"
#include "crane.hpp"
#if ENGINE_MODE
#include "gui_engine.hpp"
#include "gui_settings.hpp"
#include "compile.hpp"
#else
#include "gui_game.hpp"
#endif

using std::cout;
using std::endl;
using std::wstring;
using std::ifstream;
using std::filesystem::directory_iterator;
using std::filesystem::exists;
using std::filesystem::path;
using std::filesystem::current_path;
using std::filesystem::create_directory;
using std::system;
using std::unique_ptr;
using std::to_string;

using Utils::String;
using Utils::File;
using Graphics::Render;
using EngineFile::SceneFile;
using EngineFile::ConfigFile;
using Core::ConsoleManager;
using Caller = Core::ConsoleManager::Caller;
using Type = Core::ConsoleManager::Type;
using Graphics::Shape::GameObjectManager;
using Game::Crane;
#if ENGINE_MODE
using Graphics::GUI::EngineGUI;
using Graphics::GUI::GUISettings;
using Core::Compilation;
#else
using Graphics::GUI::GameGUI;
#endif

namespace Core
{
#if DISCORD_MODE
	unique_ptr<discord::Core> core{};
#endif
	void Engine::InitializeEngine(const string& assignedVersion)
	{
		version = assignedVersion;

#if ENGINE_MODE
		name = "Elypso engine";

		CheckForMissingCompilerFiles();
#else
		string gameFolder = path(current_path()).string();

		for (const auto& file : directory_iterator(gameFolder))
		{
			if (is_regular_file(file))
			{
				string extension = path(file).extension().string();
#ifdef _WIN32
				if (extension == ".exe")
				{
					name = path(file).stem().string();
					break;
				}
#elif __linux__
				if (path(file).extension().empty())
				{
					name = path(file).stem().string();
					break;
				}
#endif	
			}
		}
#endif

#ifdef _WIN32
		string nameAndExe = name + ".exe";
		if (IsThisProcessAlreadyRunning(nameAndExe))
		{
			CreateErrorPopup((name + " is already running!").c_str());
		}
#elif __linux__
		if (IsThisProcessAlreadyRunning(name))
		{
			CreateErrorPopup((name + " is already running!").c_str());
		}
#endif

#if ENGINE_MODE
#else
		if (name == "Elypso engine")
		{
			CreateErrorPopup("Engine cannot be ran as game!");
		}
#endif

		cout << "\n==================================================\n"
			<< "\n"
#if ENGINE_MODE
			<< "ENTERED ENGINE\n"
#else
			<< "ENTERED GAME\n"
#endif
			<< "\n"
			<< "==================================================\n"
			<< ".\n"
			<< ".\n"
			<< ".\n\n";

		ConsoleManager::WriteConsoleMessage(
			Caller::INPUT,
			Type::INFO,
			name + " " + version + "\n" +
			"Copyright (C) Lost Empire Entertainment 2025\n\n",
			true);

		//
		// SET FILES PATH
		//

		filesPath = (current_path() / "files").string();
		if (!exists(filesPath))
		{
			CreateErrorPopup("Couldn't find files folder!");
			return;
		}

		string output = name + " files path: " + filesPath + "\n\n";
		ConsoleManager::WriteConsoleMessage(
			Caller::FILE,
			Type::DEBUG,
			output);

		//
		// SET DOCUMENTS PATH
		//


#ifdef _WIN32
		PWSTR docsFolderWidePath;
		HRESULT result = SHGetKnownFolderPath(FOLDERID_Documents, 0, nullptr, &docsFolderWidePath);
		if (SUCCEEDED(result))
		{
			wstring wPath(docsFolderWidePath);
			CoTaskMemFree(docsFolderWidePath); //free the allocated memory

			//get the required buffer size
			int size_needed = WideCharToMultiByte(
				CP_UTF8,
				0,
				wPath.c_str(),
				static_cast<int>(wPath.length()),
				NULL,
				0,
				NULL,
				NULL);


			//convert wide string to utf-8 encoded narrow string
			string narrowPath(size_needed, 0);
			WideCharToMultiByte(
				CP_UTF8,
				0,
				wPath.c_str(),
				static_cast<int>(wPath.length()),
				&narrowPath[0],
				size_needed,
				NULL,
				NULL);
#if ENGINE_MODE
			docsPath = (path(narrowPath) / name).string();

			if (!exists(docsPath)) File::CreateNewFolder(docsPath);

			output = "Documents path: " + docsPath + "\n";
			ConsoleManager::WriteConsoleMessage(
				Caller::FILE,
				Type::DEBUG,
				output);
#else
			/*
			docsPath = (path(narrowPath) / "My Games").string();

			if (!exists(docsPath))
			{
				File::CreateNewFolder(docsPath);
			}

			docsPath = (path(docsPath) / name).string();
			*/
#endif
		}
		else
		{
			CreateErrorPopup("Couldn't find documents folder!");
		}
#elif __linux__
		const char* homeDir = getenv("HOME");
		if (!homeDir) 
		{
			CreateErrorPopup("HOME environment is not set!");
		}

		string documentsFolder = (path(homeDir) / "Documents").string();
		if (!exists(documentsFolder)) File::CreateNewFolder(documentsFolder);

#if ENGINE_MODE
		docsPath = (path(documentsFolder) / name).string();
#else
		docsPath = (path(documentsFolder) / "My Games" / name).string();
#endif

#endif

#if ENGINE_MODE
		//
		// SET PROJECT FILE PATH
		//

		path projectFilePath = path(docsPath) / "project.txt";
		ifstream projectFile(projectFilePath);
		if (!projectFile.is_open())
		{
			CreateErrorPopup("Failed to open project file! Did you forget to create a project in Elypso hub?");
		}

		output = "Project file path: " + projectFilePath.string() + "\n\n";
		ConsoleManager::WriteConsoleMessage(
			Caller::FILE,
			Type::DEBUG,
			output);

		string projectFileLine;
		while (getline(projectFile, projectFileLine))
		{
			if (!projectFileLine.empty())
			{
				size_t pos_project = projectFileLine.find("project:");
				if (pos_project != string::npos)
				{
					string removable = "project: ";
					size_t pos = projectFileLine.find(removable);
					projectPath = projectFileLine.erase(pos, removable.length());
				}
			}
		}
		projectFile.close();

		output = "Project path: " + projectPath + "\n\n";
		ConsoleManager::WriteConsoleMessage(
			Caller::FILE,
			Type::DEBUG,
			output);
#else
		projectPath = (current_path() / "project").string();
#endif
		//
		// SET SCENES AND TEXTURES PATHS
		//

#if ENGINE_MODE
		scenesPath = (path(projectPath) / "scenes").string();
		texturesPath = (path(projectPath) / "textures").string();

#else
		string projectName = path(projectPath).stem().string();

		path currentPath = current_path();
		scenesPath = (currentPath / "project" / "scenes").string();
		texturesPath = (currentPath / "project" / "textures").string();
#endif

		output = "Scenes path: " + scenesPath + "\n\n";
		ConsoleManager::WriteConsoleMessage(
			Caller::FILE,
			Type::DEBUG,
			output);

		output = "Textures path: " + texturesPath + "\n\n";
		ConsoleManager::WriteConsoleMessage(
			Caller::FILE,
			Type::DEBUG,
			output);

		//
		// SET FIRST SCENE PATH
		//

		string firstSceneFile = (docsPath / path(docsPath).stem() / "firstScene.txt").string();
		if (exists(firstSceneFile))
		{
			ifstream fsFile(firstSceneFile);
			if (!fsFile.is_open())
			{
				CreateErrorPopup("Failed to open first scene file!");
			}

			string line;
			while (getline(fsFile, line))
			{
				if (!line.empty()
					&& line.find("=") != string::npos)
				{
					vector<string> splitLine = String::Split(line, '=');
					string type = splitLine[0];
					string value = splitLine[1];

					//remove one space in front of value if it exists
					if (value[0] == ' ') value.erase(0, 1);
					//remove one space in front of each value comma if it exists
					for (size_t i = 0; i < value.length(); i++)
					{
						if (value[i] == ','
							&& i + 1 < value.length()
							&& value[i + 1] == ' ')
						{
							value.erase(i + 1, 1);
						}
					}

					if (type == "scene")
					{
						string parentPath = 
							(path(firstSceneFile).parent_path() / 
							path(projectPath).stem() / "scenes").string();

						gameFirstScene = (path(parentPath) / value / "scene.txt").string();
					}
				}
			}
			fsFile.close();

			bool foundExisting = false;
			for (const auto& entry : directory_iterator(scenesPath))
			{
				string entryStem = path(entry).stem().string();
				if (gameFirstScene == entryStem)
				{
					foundExisting = true;
					break;
				}
			}

			if (!foundExisting)
			{
				ConsoleManager::WriteConsoleMessage(
					Caller::FILE,
					Type::EXCEPTION,
					"Error: Failed to assign valid first scene file because the provided one does not exist! Clearing assigned value.");
				gameFirstScene = "";
			}
			else
			{
				output = "Game first scene path: " + gameFirstScene + "\n\n";
				ConsoleManager::WriteConsoleMessage(
					Caller::FILE,
					Type::DEBUG,
					output);
			}
		}

		//
		// INITIALIZE LOGGER AND LOAD CONFIG FILE
		//

		ConsoleManager::InitializeLogger();

		ConfigFile::LoadConfigFile();

		//
		// SET GAME PATHS
		//

#if ENGINE_MODE
		string gameName = ConfigFile::GetValue("gameName");

		string parentFolder = current_path().stem().string();
		//if engine is ran from repository or visual studio folder
		string msvcRelease = "x64-release";
		string msvcDebug = "x64-debug";

		if (parentFolder == msvcRelease
			|| parentFolder == msvcDebug)
		{
			gamePath = (current_path()
				.parent_path()
				.parent_path()
				.parent_path()
				.parent_path() / "Game").string();
			gameExePath = (path(gamePath) / "out" / "build" / parentFolder / gameName).string();
			gameParentPath = path(gameExePath).parent_path().string();

#ifdef _WIN32
			gameExePath = gameExePath + ".exe";
#endif
		}
		//if engine is ran from public release configuration or something else
		else
		{
			gamePath = (current_path().parent_path() / "Game").string();
			gameExePath = (path(gamePath) / "out" / "build" / msvcRelease / gameName).string();

			gameParentPath = path(gameExePath).parent_path().string();

#ifdef _WIN32
			gameExePath = gameExePath + ".exe";
#endif
		}

		output = "Game path: " + gamePath + "\n\n";
		ConsoleManager::WriteConsoleMessage(
			Caller::FILE,
			Type::DEBUG,
			output);

		output = "Game exe path: " + gameExePath + "\n\n";
		ConsoleManager::WriteConsoleMessage(
			Caller::FILE,
			Type::DEBUG,
			output);

		output = "Game parent path: " + gameParentPath + "\n\n";
		ConsoleManager::WriteConsoleMessage(
			Caller::FILE,
			Type::DEBUG,
			output);

		//if neither one works then engine cannot proceed
		if (!exists(gamePath))
		{
			CreateErrorPopup("Failed to find game template folder!");
		}
#endif

		//
		// REST OF THE INITIALIZATION
		//

		Render::RenderSetup();

		Audio::Initialize();

		//import all valid audio files
		string audioFolder = (path(Engine::projectPath) / "audio").string();
		if (!exists(audioFolder)) File::CreateNewFolder(audioFolder);
		else
		{
			for (const auto& audioFile : directory_iterator(audioFolder))
			{
				string extension = path(audioFile).extension().string();
				if (extension != ".mp3"
					&& extension != ".flac"
					&& extension != ".wav")
				{
					ConsoleManager::WriteConsoleMessage(
						Caller::FILE,
						Type::EXCEPTION,
						"Error: File '" + path(audioFile).string() + "' with extension '" + extension + "' was found in the audio folder and it is not an audio file! File will be deleted.\n");
					File::DeleteFileOrfolder(path(audioFile).string());
				}
				else
				{
					string audioFileName = path(audioFile).filename().string();
					Audio::Import(audioFileName);
				}
			}
		}

		string lastSavedScenePath = (path(Engine::docsPath) / "lastSavedScene.txt").string();
		//attempt to load last saved scene
		if (exists(lastSavedScenePath))
		{
			ifstream lastSavedSceneFile(lastSavedScenePath);
			if (!lastSavedSceneFile.is_open())
			{
				ConsoleManager::WriteConsoleMessage(
					Caller::FILE,
					Type::EXCEPTION,
					"Error: Couldn't open scene file '" + lastSavedScenePath + "'! Opening default scene.\n");
				SceneFile::LoadScene((path(scenesPath) / "Scene1" / "scene.txt").string());
			}
			else 
			{
				string line;
				string foundScenePath;
				while (getline(lastSavedSceneFile, line))
				{
					foundScenePath = line;
					break;
				}
				lastSavedSceneFile.close();

				if (exists(foundScenePath))
				{
					SceneFile::LoadScene(foundScenePath);
				}
				else 
				{
					if (foundScenePath != "")
					{
						ConsoleManager::WriteConsoleMessage(
							Caller::FILE,
							Type::EXCEPTION,
							"Error: Couldn't load scene file '" + foundScenePath + "' because it doesn't exist! Opening default scene.\n");
					}
					SceneFile::LoadScene((path(scenesPath) / "Scene1" / "scene.txt").string());
				}
			}
		}
		//otherwise load first scene
		else SceneFile::LoadScene((path(scenesPath) / "Scene1" / "scene.txt").string());

#if ENGINE_MODE
#if DISCORD_MODE
		string appID{};
		//app id specific for the engine
		if (!appID.empty())
		{
			InitializeDiscordRichPresence(appID);
		}
#endif
#endif

		SceneFile::SaveScene(SceneFile::SaveType::defaultSave, "", false);
	}

#if DISCORD_MODE
	void Engine::InitializeDiscordRichPresence(const __int64& appID)
	{
		discord::Core* rawCore{};
		discord::Result result = discord::Core::Create(
			appID,
			DiscordCreateFlags_Default, 
			&rawCore);
		if (result != discord::Result::Ok)
		{
			ConsoleManager::WriteConsoleMessage(
				Caller::INITIALIZE,
				Type::EXCEPTION,
				"Failed to initialize Discord!");
		}
		else
		{
			ConsoleManager::WriteConsoleMessage(
				Caller::INITIALIZE,
				Type::INFO,
				"Successfully initialized Discord!");
			core.reset(rawCore);
		}
	}
#endif

	void Engine::CreateErrorPopup(const char* errorMessage)
	{
		string title = name + " has shut down";

		cout << "\n"
			<< "===================="
			<< "\n"
#if ENGINE_MODE
			<< "ENGINE SHUTDOWN\n"
#else
			<< "GAME SHUTDOWN\n"
#endif
			<< "\n\n"
			<< errorMessage
			<< "\n"
			<< "===================="
			<< "\n";

#ifdef _WIN32
		int result = MessageBoxA(nullptr, errorMessage, title.c_str(), MB_ICONERROR | MB_OK);

		if (result == IDOK) Shutdown(true);
#elif __linux__
		string command = "zenity --error --text=\"" + (string)errorMessage + "\" --title=\"" + title + "\"";
		int result = system(command.c_str());
		(void)result;
		Shutdown(true);
#endif
	}

	bool Engine::IsThisProcessAlreadyRunning(const string& processName)
	{
#ifdef _WIN32
		bool processFound = false;
		HANDLE hProcessSnap;
		PROCESSENTRY32 pe32{};
		
		DWORD currentProcessId = GetCurrentProcessId();

		//take a snapshot of all processes
		hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
		if (hProcessSnap == INVALID_HANDLE_VALUE)
		{
			cout << "Error: CreateToolhelp32Snapshot failed!\n";
			return false;
		}

		pe32.dwSize = sizeof(PROCESSENTRY32);

		if (!Process32First(hProcessSnap, &pe32))
		{
			cout << "Error: Process32First failed!\n";
			CloseHandle(hProcessSnap);
			return false;
		}

		do
		{
			//compare the current process name with the one to check
			if (strcmp(pe32.szExeFile, processName.c_str()) == 0)
			{
				//check if this is not the current process
				if (pe32.th32ProcessID != currentProcessId)
				{
					processFound = true;
					break;
				}
			}
		} while (Process32Next(hProcessSnap, &pe32));

		CloseHandle(hProcessSnap);
		return processFound;
#elif __linux__
		//get the process id
		pid_t currentPID = getpid();

		//construct the command to find processes by name, excluding the currend PID
		string command = "pgrep -x \"" + processName + "\" | grep -v " + to_string(currentPID) + " > /dev/null";

		//execute the command and return the result
		return (system(command.c_str()) == 0);
#endif
		return false;
	}

	void Engine::RunEngine()
	{
		ConsoleManager::WriteConsoleMessage(
			Caller::INPUT,
			Type::DEBUG,
			"Reached window loop successfully!\n\n");

		ConsoleManager::WriteConsoleMessage(
			Caller::INPUT,
			Type::INFO,
			"==================================================\n\n",
			true,
			false);

		isEngineRunning = true;
#if ENGINE_MODE
		if (!ConsoleManager::storedLogs.empty())
		{
			ConsoleManager::PrintLogsToBuffer();
		}
#endif

		while (isEngineRunning)
		{
			TimeManager::UpdateDeltaTime();

			if (Crane::crane == nullptr
				&& Crane::crane_grab == nullptr
				&& Crane::craneController == nullptr)
			{
				Crane::LookForCraneAndCraneController();
			}
			Crane::MoveCrane();
			Crane::GrabBarrel();

			Render::WindowLoop();
#if DISCORD_MODE
			RunDiscordRichPresence();
#endif
			if (glfwWindowShouldClose(Render::window))
			{
				isEngineRunning = false;
			}
		}
	}
#if DISCORD_MODE
	void Engine::RunDiscordRichPresence()
	{
		if (core) core->RunCallbacks();
	}
	void Engine::SetDiscordRichPresence(
		const string& state,
		const string& details,
		const time_t& time_start,
		const time_t& time_end,
		const string& largeImage,
		const string& largeText,
		const string& smallImage,
		const string& smallText)
	{
		if (!core) return;

		discord::Activity activity{};

		//set Rich Presence details
		activity.SetState(state.c_str());
		activity.SetDetails(details.c_str());

		//starts counting from when discord rich presence 
		//was called for first time until application closes
		if (time_start == 0)
		{
			static bool setCurrentTimeOnce = false;

			if (!setCurrentTimeOnce)
			{
				activity.GetTimestamps().SetStart(time(nullptr));
				setCurrentTimeOnce = true;
			}
		}
		//starts counting from assigned time
		else if (time_start > 0) activity.GetTimestamps().SetStart(time_start);

		//ends counting at assigned time
		if (time_end != 0) activity.GetTimestamps().SetEnd(time_end);

		//set large and small images
		if (largeImage != "") activity.GetAssets().SetLargeImage(largeImage.c_str());
		if (largeText != "") activity.GetAssets().SetLargeText(largeText.c_str());
		if (smallImage != "") activity.GetAssets().SetSmallImage(smallImage.c_str());
		if (smallText != "") activity.GetAssets().SetSmallText(smallText.c_str());

		//update the activity via the ActivityManager
		core->ActivityManager().UpdateActivity(activity, [](discord::Result result) {});
	}
#endif

#if ENGINE_MODE
	void Engine::CheckForMissingCompilerFiles()
	{
#ifdef _WIN32
		string msvcCommand = "call \"C:\\Program Files\\Microsoft Visual Studio\\2022\\Community\\VC\\Auxiliary\\Build\\vcvarsall.bat\" x64 >nul 2>&1 && cl >nul 2>&1";

		int msvcResult = system(msvcCommand.c_str());

		if (msvcResult != 0)
		{
			CreateErrorPopup("Couldn't run engine because of missing MSVC or C++ components! Please set up MSVC and C++ components for desktop and game development.");
		}

		string cmakeCommand = "cmake --version >nul 2>&1";

		int cmakeResult = system(cmakeCommand.c_str());

		if (cmakeResult != 0)
		{
			CreateErrorPopup("Couldn't run engine because of missing CMAKE installer! Please install CMake and add it to environment path.");
		}
#endif
	}
#endif

	//counts as idle if minimized
	bool Engine::IsUserIdle()
	{
		//checks if window is minimized
		int width, height;
		glfwGetWindowSize(Render::window, &width, &height);
		if (width == 0 || height == 0) return true;

		else return false;
	}

	void Engine::Shutdown(bool immediate)
	{
		if (immediate)
		{
			isEngineRunning = false;

			ConsoleManager::CloseLogger();
#if ENGINE_MODE
			EngineGUI::Shutdown();
#else
			GameGUI::Shutdown();
#endif
			glfwTerminate();

			quick_exit(EXIT_SUCCESS);
		}
		else
		{
			if (SceneFile::unsavedChanges == true)
			{
#if ENGINE_MODE
				//unminimize and bring to focus to ensure the user always sees the shutdown confirmation popup
				if (glfwGetWindowAttrib(Render::window, GLFW_ICONIFIED))
				{
					glfwRestoreWindow(Render::window);
				}
				glfwFocusWindow(Render::window);

				glfwSetWindowShouldClose(Render::window, GLFW_FALSE);
				EngineGUI::renderUnsavedShutdownWindow = true;
#else
				SceneFile::unsavedChanges = false;
				Shutdown();
#endif
			}
			else
			{
				isEngineRunning = false;

				ConfigFile::SaveConfigFile();

				ConsoleManager::WriteConsoleMessage(
					Caller::INPUT,
					Type::INFO,
					"==================================================\n\n",
					true);

				ConsoleManager::WriteConsoleMessage(
					Caller::SHUTDOWN,
					Type::INFO,
					"Cleaning up resources...\n");

				Audio::Shutdown();
#if ENGINE_MODE
				EngineGUI::Shutdown();
#else
				GameGUI::Shutdown();
#endif
				ConsoleManager::WriteConsoleMessage(
					Caller::SHUTDOWN,
					Type::INFO,
					"Shutdown complete!\n");

				cout << "\n==================================================\n"
					<< "\n"
#if ENGINE_MODE
					<< "EXITED ENGINE\n"
#else
					<< "EXITED GAME\n"
#endif
					<< "\n"
					<< "==================================================\n"
					<< ".\n"
					<< ".\n"
					<< ".\n\n";

				ConsoleManager::CloseLogger();

				glfwTerminate();

				exit(EXIT_SUCCESS);
			}
		}
	}
}
