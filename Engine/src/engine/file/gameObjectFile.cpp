//Copyright(C) 2025 Lost Empire Entertainment
//This program comes with ABSOLUTELY NO WARRANTY.
//This is free software, and you are welcome to redistribute it under certain conditions.
//Read LICENSE.md for more information.

#include <fstream>
#include <filesystem>
#include <iostream>
#include <unordered_map>
#include <memory>
#include <vector>

//external
#include "magic_enum.hpp"
#include "glm.hpp"

//engine
#include "gameObjectFile.hpp"
#include "core.hpp"
#include "console.hpp"
#include "fileUtils.hpp"
#include "stringUtils.hpp"
#include "importer.hpp"
#include "pointlight.hpp"
#include "spotlight.hpp"
#include "directionallight.hpp"
#include "empty.hpp"
#include "render.hpp"
#include "selectobject.hpp"
#include "gameobject.hpp"
#include "texture.hpp"
#include "transformcomponent.hpp"
#include "meshcomponent.hpp"
#include "materialcomponent.hpp"
#include "lightcomponent.hpp"
#include "audioplayercomponent.hpp"
#if ENGINE_MODE
#include "gui_scenewindow.hpp"
#endif

using std::ifstream;
using std::ofstream;
using std::filesystem::exists;
using std::filesystem::path;
using std::filesystem::directory_iterator;
using glm::vec3;
using std::filesystem::is_empty;
using std::filesystem::is_directory;
using std::filesystem::is_regular_file;
using std::filesystem::current_path;
using std::make_shared;
using std::cout;
using std::to_string;
using std::unordered_map;
using std::shared_ptr;
using std::vector;

using Core::Engine;
using Core::ConsoleManager;
using Caller = Core::ConsoleManager::Caller;
using Type = Core::ConsoleManager::Type;
using Utils::File;
using Utils::String;
using Graphics::Components::TransformComponent;
using Graphics::Components::MeshComponent;
using Graphics::Components::MaterialComponent;
using Graphics::Components::LightComponent;
using MeshType = Graphics::Components::MeshComponent::MeshType;
using Graphics::Shape::Importer;
using Graphics::Shape::PointLight;
using Graphics::Shape::SpotLight;
using Graphics::Shape::DirectionalLight;
using Graphics::Shape::Empty;
using Core::Select;
using Graphics::Render;
using Graphics::Shape::GameObject;
using Graphics::Shape::GameObjectManager;
using Graphics::Texture;
using Graphics::Shader;
using Graphics::Shape::GameObject;
using Graphics::Components::AudioPlayerComponent;
#if ENGINE_MODE
using Graphics::GUI::GUISceneWindow;
#endif

namespace EngineFile
{
	void GameObjectFile::SaveGameObjects()
	{
		for (const auto& obj : GameObjectManager::GetObjects())
		{
			if (obj->GetParentBillboardHolder() == nullptr)
			{
				string objectName = obj->GetName();
				string txtName = objectName + ".txt";
				string objectTxtFilePath = (path(Engine::scenePath).parent_path() / "gameobjects" / objectName / txtName).string();

				vector<string> data;

				data.push_back("name= " + obj->GetName() + "\n");

				data.push_back("id= " + to_string(obj->GetID()) + "\n");

				data.push_back("enabled= " + to_string(obj->IsEnabled()) + "\n");

				//
				// TRANSFORM DATA
				//

				//position
				float posX = obj->GetTransform()->GetPosition().x;
				float posY = obj->GetTransform()->GetPosition().y;
				float posZ = obj->GetTransform()->GetPosition().z;
				data.push_back(
					"position= " + to_string(posX) + ", "
					+ to_string(posY) + ", "
					+ to_string(posZ) + "\n");

				//rotation
				float rotX = obj->GetTransform()->GetRotation().x;
				float rotY = obj->GetTransform()->GetRotation().y;
				float rotZ = obj->GetTransform()->GetRotation().z;
				data.push_back(
					"rotation= " + to_string(rotX) + ", "
					+ to_string(rotY) + ", "
					+ to_string(rotZ) + "\n");

				//scale
				float scaleX = obj->GetTransform()->GetScale().x;
				float scaleY = obj->GetTransform()->GetScale().y;
				float scaleZ = obj->GetTransform()->GetScale().z;
				data.push_back(
					"scale= " + to_string(scaleX) + ", "
					+ to_string(scaleY) + ", "
					+ to_string(scaleZ) + "\n");

				data.push_back("\n");

				auto mesh = obj->GetComponent<MeshComponent>();
				if (mesh)
				{
					//
					// MESH DATA
					//

					string type = string(magic_enum::enum_name(mesh->GetMeshType()));
					data.push_back("type= " + type + "\n");

					//path to txt file of this gameobject
					data.push_back("txtFile= " + obj->GetTxtFilePath() + "\n");

					//
					// MATERIAL DATA
					//

					//object textures
					MeshComponent::MeshType meshType = mesh->GetMeshType();
					auto mat = obj->GetComponent<MaterialComponent>();
					if (meshType == MeshComponent::MeshType::model)
					{
						string diffuseTexture = mat->GetTextureName(MaterialComponent::TextureType::diffuse);
						diffuseTexture = path(diffuseTexture).filename().string();
						if (diffuseTexture == "diff_default.png") diffuseTexture = "DEFAULTDIFF";

						string specularTexture = mat->GetTextureName(MaterialComponent::TextureType::specular);
						specularTexture = path(specularTexture).filename().string();
						if (specularTexture == "spec_default.png") specularTexture = "DEFAULTSPEC";

						string normalTexture = mat->GetTextureName(MaterialComponent::TextureType::normal);
						normalTexture = path(normalTexture).filename().string();

						string heightTexture = mat->GetTextureName(MaterialComponent::TextureType::height);
						heightTexture = path(heightTexture).filename().string();

						data.push_back(
							+"textures= "
							+ diffuseTexture + ", "
							+ specularTexture + ", "
							+ normalTexture + ", "
							+ heightTexture + "\n");
					}

					if (meshType == MeshComponent::MeshType::model)
					{
						data.push_back("shininess= " + to_string(32) + "\n");
					}

					//
					// LIGHT DATA
					//

					else if (meshType == MeshComponent::MeshType::point_light)
					{
						auto light = obj->GetComponent<LightComponent>();
						if (light)
						{
							float pointDiffuseX = light->GetDiffuse().x;
							float pointDiffuseY = light->GetDiffuse().y;
							float pointDiffuseZ = light->GetDiffuse().z;
							data.push_back(
								"diffuse= " + to_string(pointDiffuseX) + ", "
								+ to_string(pointDiffuseY) + ", "
								+ to_string(pointDiffuseZ) + "\n");

							data.push_back("intensity= " + to_string(light->GetIntensity()) + "\n");

							data.push_back("distance= " + to_string(light->GetDistance()) + "\n");
						}
					}
					else if (meshType == MeshComponent::MeshType::spot_light)
					{
						auto light = obj->GetComponent<LightComponent>();
						if (light)
						{
							float spotDiffuseX = light->GetDiffuse().x;
							float spotDiffuseY = light->GetDiffuse().y;
							float spotDiffuseZ = light->GetDiffuse().z;
							data.push_back(
								"diffuse= " + to_string(spotDiffuseX) + ", "
								+ to_string(spotDiffuseY) + ", "
								+ to_string(spotDiffuseZ) + "\n");

							data.push_back("intensity= " + to_string(light->GetIntensity()) + "\n");

							data.push_back("distance= " + to_string(light->GetDistance()) + "\n");

							data.push_back("inner angle= " + to_string(light->GetInnerAngle()) + "\n");

							data.push_back("outer angle= " + to_string(light->GetOuterAngle()) + "\n");
						}
					}
					else if (meshType == MeshComponent::MeshType::directional_light)
					{
						auto light = obj->GetComponent<LightComponent>();
						if (light)
						{
							float dirDiffuseX = light->GetDiffuse().x;
							float dirDiffuseY = light->GetDiffuse().y;
							float dirDiffuseZ = light->GetDiffuse().z;
							data.push_back(
								"diffuse= " + to_string(dirDiffuseX) + ", "
								+ to_string(dirDiffuseY) + ", "
								+ to_string(dirDiffuseZ) + "\n");

							data.push_back("intensity= " + to_string(light->GetIntensity()) + "\n");
						}
					}

					//also save billboard data of each light source
					if (meshType == MeshComponent::MeshType::point_light
						|| meshType == MeshComponent::MeshType::spot_light
						|| meshType == MeshComponent::MeshType::directional_light)
					{
						data.push_back("\n");
						data.push_back("---attached billboard data---\n");
						data.push_back("\n");

						data.push_back("billboard id= " + to_string(obj->GetChildBillboard()->GetID()) + "\n");

						data.push_back("billboard enabled= " + to_string(obj->GetChildBillboard()->IsEnabled()) + "\n");
					}
				}

				//
				// AUDIO PLAYER DATA
				//

				auto apc = obj->GetComponent<AudioPlayerComponent>();
				if (apc)
				{
					string audioFileName = apc->GetName();
					bool is3D = apc->Is3D();
					float currVolume = apc->GetVolume();

					data.push_back("audioFileName= " + audioFileName + "\n");
					data.push_back("is3D= " + to_string(is3D) + "\n");
					data.push_back("currentVolume= " + to_string(currVolume) + "\n");
				}

				//
				// WRITE ALL DATA INTO NEW TXT FILE
				//

				string folderPath = (path(Engine::scenePath).parent_path() / "gameobjects" / objectName).string();
				if (!exists(folderPath))
				{
					File::CreateNewFolder(folderPath);
				}

				if (exists(objectTxtFilePath)) File::DeleteFileOrfolder(objectTxtFilePath);

				ofstream objectFile(objectTxtFilePath);

				if (!objectFile.is_open())
				{
					ConsoleManager::WriteConsoleMessage(
						Caller::FILE,
						Type::EXCEPTION,
						"Error: Couldn't write into object txt file path '" + objectTxtFilePath + "'!\n");
					return;
				}

				for (const string dataLine : data)
				{
					objectFile << dataLine;
				}

				objectFile.close();

				ConsoleManager::WriteConsoleMessage(
					Caller::FILE,
					Type::DEBUG,
					"Successfully saved gameobject " + obj->GetName() + " with ID " + to_string(obj->GetID()) + ".\n");
			}
		}
	}

	string GameObjectFile::GetType(const string& file)
	{
		ifstream objFile(file);

		if (!objFile.is_open())
		{
			ConsoleManager::WriteConsoleMessage(
				Caller::FILE,
				Type::EXCEPTION,
				"Error: Couldn't read from object file '" + file + "'!\n");
			return "";
		}

		string target = "type= ";
		string line;
		while (getline(objFile, line))
		{
			size_t pos = line.find(target);
			if (pos != string::npos)
			{
				return line.substr(pos + target.length());
			}
		}
		objFile.close();

		return "";
	}

	void GameObjectFile::LoadGameObjects()
	{
		if (Engine::currentGameobjectsPath.empty())
		{
			ConsoleManager::WriteConsoleMessage(
				Caller::FILE,
				Type::DEBUG,
				"There were no gameobjects to load for scene '" + path(Engine::scenePath).parent_path().stem().string() + "'.\n");

			return;
		}

		ConsoleManager::WriteConsoleMessage(
			Caller::FILE,
			Type::DEBUG,
			"Started loading gameobjects for scene '" + path(Engine::scenePath).parent_path().stem().string() + "'.\n");
#if ENGINE_MODE
		GUISceneWindow::waitBeforeCountsUpdate = true;
#endif
		vector<string> validAssimpPaths;
		vector<string> validGameobjectPaths;

		//look through parent gameobjects folder
		for (const auto& folder : directory_iterator(Engine::currentGameobjectsPath))
		{
			//look for assimp model files (fbx, glfw, obj)
			for (const auto& file : directory_iterator(folder))
			{
				string extension = path(file).extension().string();
				if (extension == ".fbx"
					|| extension == ".obj"
					|| extension == ".glfw")
				{
					validAssimpPaths.push_back(path(file).string());
				}
			}
			//look through child gameobject folders
			for (const auto& file : directory_iterator(folder))
			{
				//look for light txt files
				if (is_regular_file(file))
				{
					string extension = path(file).extension().string();
					if (extension == ".txt")
					{
						validGameobjectPaths.push_back(path(file).string());
					}
				}
			}
		}

		//loads all assimp models
		for (const auto& assimpPath : validAssimpPaths)
		{
			string fileName = path(assimpPath).stem().string();
			string modelPath = path(assimpPath).string();

			string sceneName = path(Engine::scenePath).parent_path().stem().string();
			string parentFolderName = path(modelPath).parent_path().stem().string();
			string txtFile = parentFolderName + ".txt";

			string txtPath = (path(Engine::projectPath) / "scenes" / sceneName / "gameobjects" / parentFolderName / txtFile).string();
			string type = GetType(txtPath);
			if (type == "")
			{
				Empty::InitializeEmpty(
					vec3(0),
					vec3(0),
					vec3(1),
					modelPath,
					fileName,
					Empty::tempID,
					true);
			}
			else
			{
				Importer::Initialize(
					vec3(0),
					vec3(0),
					vec3(1),
					modelPath,
					"DEFAULTDIFF",
					"DEFAULTSPEC",
					"EMPTY",
					"EMPTY",
					fileName,
					Importer::tempID);
			}
		}

		//loads all light txt files
		for (const auto& gameobjectPath : validGameobjectPaths)
		{
			string filePath = path(gameobjectPath).string();
			string type = GetType(filePath);
			if (type == "point_light")
			{
				LoadPointLight(filePath);
			}
			else if (type == "spot_light")
			{
				LoadSpotlight(filePath);
			}
			else if (type == "directional_light")
			{
				LoadDirectionalLight(filePath);
			}
		}
#if ENGINE_MODE
		GUISceneWindow::waitBeforeCountsUpdate = false;
		GUISceneWindow::UpdateCounts();
#endif
		ConsoleManager::WriteConsoleMessage(
			Caller::FILE,
			Type::DEBUG,
			"Finished loading gameobjects for scene '" + path(Engine::scenePath).parent_path().stem().string() + "'.\n");
	}

	void GameObjectFile::LoadModel(const string& txtFilePath)
	{
#if ENGINE_MODE
		string fullTxtFilePath = (path(Engine::projectPath) / txtFilePath).string();
#else
		string fullTxtFilePath = (current_path() / "project" / txtFilePath).string();
#endif
		//skip running this function for newly imported models that dont actually
		//have a txt file yet even though their txt file path was assigned
		if (!exists(fullTxtFilePath)) return;

		//
		// READ FROM MODEL FILE
		//

		ifstream txtFile(fullTxtFilePath);
		if (!txtFile.is_open())
		{
			ConsoleManager::WriteConsoleMessage(
				Caller::FILE,
				Type::EXCEPTION,
				"Error: Failed to open model txt file '" + fullTxtFilePath + "'!\n\n");
			return;
		}

		unordered_map<string, string> data;
		string line;
		while (getline(txtFile, line))
		{
			if (!line.empty()
				&& line.find("=") != string::npos)
			{
				vector<string> splitLine = String::Split(line, '=');
				string key = splitLine[0];
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

				if (key == "name"
					|| key == "id"
					|| key == "enabled"
					|| key == "type"
					|| key == "position"
					|| key == "rotation"
					|| key == "scale"

					|| key == "textures"
					|| key == "model"
					|| key == "shininess"
					|| key == "audioFileName"
					|| key == "is3D"
					|| key == "currentVolume")
				{
					data[key] = value;
				}
			}
		}

		txtFile.close();

		//
		// ASSIGN MODEL DATA TO VARIABLES
		//

		string name{};
		unsigned int ID{};
		bool isEnabled{};
		MeshComponent::MeshType type{};
		vec3 pos{};
		vec3 rot{};
		vec3 scale{};

		vector<string> textures{};
		string model{};
		float shininess{};

		string audioFileName{};
		bool is3D{};
		float currVolume{};

		for (const auto& [key, value] : data)
		{
			if (key == "name")
			{
				name = value;
			}
			else if (key == "id")
			{
				ID = stoul(value);
			}
			else if (key == "enabled")
			{
				isEnabled = stoi(value);
			}
			else if (key == "type")
			{
				auto typeAuto = magic_enum::enum_cast<MeshComponent::MeshType>(value);
				if (typeAuto.has_value())
				{
					type = typeAuto.value();
				}
			}
			else if (key == "position")
			{
				vector<string> split = String::Split(value, ',');
				vec3 newPos = vec3(stof(split[0]), stof(split[1]), stof(split[2]));
				pos = newPos;
			}
			else if (key == "rotation")
			{
				vector<string> split = String::Split(value, ',');
				vec3 newRot = vec3(stof(split[0]), stof(split[1]), stof(split[2]));
				rot = newRot;
			}
			else if (key == "scale")
			{
				vector<string> split = String::Split(value, ',');
				vec3 newScale = vec3(stof(split[0]), stof(split[1]), stof(split[2]));
				scale = newScale;
			}

			else if (key == "textures")
			{
				vector<string> split = String::Split(value, ',');

				string fullTex0Path = split[0] == "DEFAULTDIFF"
					? "DEFAULTDIFF"
					: (path(fullTxtFilePath).parent_path() / split[0]).string();

				if (fullTex0Path == "DEFAULTDIFF")
				{
					fullTex0Path = (path(Engine::filesPath) / "textures" / "diff_default.png").string();
					textures.push_back(fullTex0Path);
				}
				else if (fullTex0Path != "DEFAULTDIFF"
						 && !exists(fullTex0Path))
				{
					ConsoleManager::WriteConsoleMessage(
						Caller::FILE,
						Type::EXCEPTION,
						"Error: Texture at slot 0 for " + name + " at " + fullTex0Path + " does not exist!\n");
					textures.push_back((path(Engine::filesPath) / "textures" / "diff_missing.png").string());
				}
				else textures.push_back(fullTex0Path);

				string fullTex1Path = split[1] == "DEFAULTSPEC"
					? "DEFAULTSPEC"
					: (path(fullTxtFilePath).parent_path() / split[1]).string();
				if (fullTex1Path == "DEFAULTSPEC")
				{
					fullTex1Path = (path(Engine::filesPath) / "textures" / "spec_default.png").string();
					textures.push_back(fullTex1Path);
				}
				else if (fullTex1Path != "DEFAULTSPEC"
					&& !exists(fullTex1Path))
				{
					ConsoleManager::WriteConsoleMessage(
						Caller::FILE,
						Type::EXCEPTION,
						"Error: Texture at slot 1 for " + name + " at " + fullTex1Path + " does not exist!\n");
					textures.push_back("DEFAULTSPEC");
				}
				else textures.push_back(fullTex1Path);

				string fullTex2Path = split[2] == "EMPTY"
					? "EMPTY"
					: (path(fullTxtFilePath).parent_path() / split[2]).string();
				if (fullTex2Path != "EMPTY"
					&& !exists(fullTex2Path))
				{
					ConsoleManager::WriteConsoleMessage(
						Caller::FILE,
						Type::EXCEPTION,
						"Error: Texture at slot 2 for " + name + " at " + fullTex2Path + " does not exist!\n");
					textures.push_back("EMPTY");
				}
				else textures.push_back(fullTex2Path);

				string fullTex3Path = split[3] == "EMPTY"
					? "EMPTY"
					: (path(fullTxtFilePath).parent_path() / split[3]).string();
				if (fullTex3Path != "EMPTY"
					&& !exists(fullTex3Path))
				{
					ConsoleManager::WriteConsoleMessage(
						Caller::FILE,
						Type::EXCEPTION,
						"Error: Texture at slot 3 for " + name + " at " + fullTex3Path + " does not exist!\n");
					textures.push_back("EMPTY");
				}
				else textures.push_back(fullTex3Path);
			}
			else if (key == "model")
			{
				model = value;
			}
			else if (key == "shininess")
			{
				shininess = stof(value);
			}
			else if (key == "audioFileName")
			{
				audioFileName = value;
			}
			else if (key == "is3D")
			{
				is3D = stoi(value);
			}
			else if (key == "currentVolume")
			{
				currVolume = stof(value);
			}
		}

		//
		// SET UP TEXTURES FOR MODEL
		//

		string diff_missing = (path(Engine::filesPath) / "textures" / "diff_missing.png").string();
		string diffuseTexture = textures[0];
		if (diffuseTexture != "EMPTY"
			&& !exists(diffuseTexture))
		{
			ConsoleManager::WriteConsoleMessage(
				Caller::FILE,
				Type::EXCEPTION,
				"Error: Diffuse texture " + diffuseTexture + " for " + name + " not found!\n");
			diffuseTexture = diff_missing;
		}

		string specularTexture = textures[1];
		if (specularTexture != "EMPTY"
			&& !exists(specularTexture))
		{
			ConsoleManager::WriteConsoleMessage(
				Caller::FILE,
				Type::EXCEPTION,
				"Error: Specular texture " + specularTexture + " for " + name + " not found!\n");
			specularTexture = "EMPTY";
		}

		string normalTexture = textures[2];
		if (normalTexture != "EMPTY"
			&& !exists(normalTexture))
		{
			ConsoleManager::WriteConsoleMessage(
				Caller::FILE,
				Type::EXCEPTION,
				"Error: Normal texture " + normalTexture + " for " + name + " not found!\n");
			normalTexture = "EMPTY";
		}

		string heightTexture = textures[3];
		if (heightTexture != "EMPTY"
			&& !exists(heightTexture))
		{
			ConsoleManager::WriteConsoleMessage(
				Caller::FILE,
				Type::EXCEPTION,
				"Error: Height texture " + heightTexture + " for " + name + " not found!\n");
			heightTexture = "EMPTY";
		}

		//
		// LOAD MODEL
		//
 
		shared_ptr<GameObject> foundObj;
		for (const auto& obj : GameObjectManager::GetObjects())
		{
			if (obj->GetName() == name)
			{
				foundObj = obj;
				break;
			}
		}

		if (foundObj == nullptr)
		{
			ConsoleManager::WriteConsoleMessage(
				Caller::FILE,
				Type::EXCEPTION,
				"Error: Tried to apply data to scene model '" + name + "' with ID '" + to_string(ID) + "' but it does not exist!\n");
		}
		else
		{
			ConsoleManager::WriteConsoleMessage(
				Caller::FILE,
				Type::DEBUG,
				"Loading model '" + name + "' with ID '" + to_string(ID) + "' for scene '" + path(Engine::scenePath).parent_path().stem().string() + "'.\n");

			foundObj->SetName(name);
			foundObj->SetID(ID);
			foundObj->SetEnableState(isEnabled);

			foundObj->GetTransform()->SetPosition(pos);
			foundObj->GetTransform()->SetRotation(rot);
			foundObj->GetTransform()->SetScale(scale);

			string vert = (path(Engine::filesPath) / "shaders" / "Gameobject.vert").string();
			string frag = (path(Engine::filesPath) / "shaders" / "Gameobject.frag").string();
			Shader modelShader = Shader::LoadShader(vert, frag);
			auto mat = foundObj->AddComponent<MaterialComponent>();
			mat->AddShader(vert, frag, modelShader);

			Texture::LoadTexture(foundObj, diffuseTexture, MaterialComponent::TextureType::diffuse, false);
			Texture::LoadTexture(foundObj, specularTexture, MaterialComponent::TextureType::specular, false);
			Texture::LoadTexture(foundObj, normalTexture, MaterialComponent::TextureType::height, false);
			Texture::LoadTexture(foundObj, heightTexture, MaterialComponent::TextureType::normal, false);

			if (audioFileName != "")
			{
				auto apc = foundObj->AddComponent<AudioPlayerComponent>();
				apc->SetName(audioFileName);
				apc->Set3DState(is3D);
				apc->SetVolume(currVolume);
			}

			/*
			* 
			* SHININESS VARIABLE IS CURRENTLY UNUSED
			* IT WILL BE RE-ENABLED IN THE FUTURE
			* 
			foundObj->GetBasicShape()->SetShininess(shininess);
			*/

			GameObject::nextID = ID + 1;
		}
	}

	void GameObjectFile::LoadPointLight(const string& file)
	{
		//
		// READ FROM POINT LIGHT FILE
		//

		ifstream pointLightFile(file);
		if (!pointLightFile.is_open())
		{
			ConsoleManager::WriteConsoleMessage(
				Caller::FILE,
				Type::EXCEPTION,
				"Error: Failed to open point light file '" + file + "'!\n\n");
			return;
		}

		unordered_map<string, string> data;
		string line;
		while (getline(pointLightFile, line))
		{
			if (!line.empty()
				&& line.find("=") != string::npos)
			{
				vector<string> splitLine = String::Split(line, '=');
				string key = splitLine[0];
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

				if (key == "name"
					|| key == "id"
					|| key == "enabled"
					|| key == "type"
					|| key == "position"
					|| key == "rotation"
					|| key == "scale"

					|| key == "txtFile"

					|| key == "diffuse"
					|| key == "intensity"
					|| key == "distance"

					|| key == "billboard id"
					|| key == "billboard enabled")
				{
					data[key] = value;
				}
			}
		}

		pointLightFile.close();

		//
		// ASSIGN POINT LIGHT DATA TO VARIABLES
		//

		string name{};
		unsigned int ID{};
		bool isEnabled{};
		MeshComponent::MeshType type{};
		vec3 pos{};
		vec3 rot{};
		vec3 scale{};

		string txtFile{};

		vec3 diffuse{};
		float intensity{};
		float distance{};

		unsigned int billboardID{};
		bool isBillboardEnabled{};

		for (const auto& [key, value] : data)
		{
			if (key == "name")
			{
				name = value;
			}
			else if (key == "id")
			{
				ID = stoul(value);
			}
			else if (key == "enabled")
			{
				isEnabled = stoi(value);
			}
			else if (key == "type")
			{
				auto typeAuto = magic_enum::enum_cast<MeshComponent::MeshType>(value);
				if (typeAuto.has_value())
				{
					type = typeAuto.value();
				}
			}
			else if (key == "position")
			{
				vector<string> split = String::Split(value, ',');
				vec3 newPos = vec3(stof(split[0]), stof(split[1]), stof(split[2]));
				pos = newPos;
			}
			else if (key == "rotation")
			{
				vector<string> split = String::Split(value, ',');
				vec3 newRot = vec3(stof(split[0]), stof(split[1]), stof(split[2]));
				rot = newRot;
			}
			else if (key == "scale")
			{
				vector<string> split = String::Split(value, ',');
				vec3 newScale = vec3(stof(split[0]), stof(split[1]), stof(split[2]));
				scale = newScale;
			}

			else if (key == "txtFile")
			{
				txtFile = value;
			}

			else if (key == "diffuse")
			{
				vector<string> split = String::Split(value, ',');
				vec3 newDiffuse = vec3(stof(split[0]), stof(split[1]), stof(split[2]));
				diffuse = newDiffuse;
			}
			else if (key == "intensity") intensity = stof(value);
			else if (key == "distance") distance = stof(value);

			else if (key == "billboard id")
			{
				billboardID = stoul(value);
			}
			else if (key == "billboard enabled")
			{
				isBillboardEnabled = stoi(value);
			}
		}

		//
		// LOAD POINT LIGHT
		//

		ConsoleManager::WriteConsoleMessage(
			Caller::FILE,
			Type::DEBUG,
			"Loading point light '" + name + "' with ID '" + to_string(ID) + "' for scene '" + path(Engine::scenePath).parent_path().stem().string() + "'.\n");

		PointLight::InitializePointLight(
			pos,
			rot,
			scale,
			txtFile,
			diffuse,
			intensity,
			distance,
			name,
			ID,
			isEnabled,
			billboardID,
			isBillboardEnabled);

		GameObject::nextID = ID + 1;
	}

	void GameObjectFile::LoadSpotlight(const string& file)
	{
		//
		// READ FROM SPOTLIGHT FILE
		//

		ifstream spotLightFile(file);
		if (!spotLightFile.is_open())
		{
			ConsoleManager::WriteConsoleMessage(
				Caller::FILE,
				Type::EXCEPTION,
				"Error: Failed to open spotlight file '" + file + "'!\n\n");
			return;
		}

		unordered_map<string, string> data;
		string line;
		while (getline(spotLightFile, line))
		{
			if (!line.empty()
				&& line.find("=") != string::npos)
			{
				vector<string> splitLine = String::Split(line, '=');
				string key = splitLine[0];
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

				if (key == "name"
					|| key == "id"
					|| key == "enabled"
					|| key == "type"
					|| key == "position"
					|| key == "rotation"
					|| key == "scale"

					|| key == "txtFile"

					|| key == "diffuse"
					|| key == "intensity"
					|| key == "distance"
					|| key == "inner angle"
					|| key == "outer angle"

					|| key == "billboard id"
					|| key == "billboard enabled")
				{
					data[key] = value;
				}
			}
		}

		spotLightFile.close();

		//
		// ASSIGN SPOTLIGHT DATA TO VARIABLES
		//

		string name{};
		unsigned int ID{};
		bool isEnabled{};
		MeshComponent::MeshType type{};
		vec3 pos{};
		vec3 rot{};
		vec3 scale{};

		string txtFile{};

		vec3 diffuse{};
		float intensity{};
		float distance{};
		float innerAngle{};
		float outerAngle{};

		unsigned int billboardID{};
		bool isBillboardEnabled{};

		for (const auto& [key, value] : data)
		{
			if (key == "name")
			{
				name = value;
			}
			else if (key == "id")
			{
				ID = stoul(value);
			}
			else if (key == "enabled")
			{
				isEnabled = stoi(value);
			}
			else if (key == "type")
			{
				auto typeAuto = magic_enum::enum_cast<MeshComponent::MeshType>(value);
				if (typeAuto.has_value())
				{
					type = typeAuto.value();
				}
			}
			else if (key == "position")
			{
				vector<string> split = String::Split(value, ',');
				vec3 newPos = vec3(stof(split[0]), stof(split[1]), stof(split[2]));
				pos = newPos;
			}
			else if (key == "rotation")
			{
				vector<string> split = String::Split(value, ',');
				vec3 newRot = vec3(stof(split[0]), stof(split[1]), stof(split[2]));
				rot = newRot;
			}
			else if (key == "scale")
			{
				vector<string> split = String::Split(value, ',');
				vec3 newScale = vec3(stof(split[0]), stof(split[1]), stof(split[2]));
				scale = newScale;
			}

			else if (key == "txtFile")
			{
				txtFile = value;
			}

			else if (key == "diffuse")
			{
				vector<string> split = String::Split(value, ',');
				vec3 newDiffuse = vec3(stof(split[0]), stof(split[1]), stof(split[2]));
				diffuse = newDiffuse;
			}
			else if (key == "intensity") intensity = stof(value);
			else if (key == "distance") distance = stof(value);
			else if (key == "inner angle") innerAngle = stof(value);
			else if (key == "outer angle") outerAngle = stof(value);

			else if (key == "billboard id")
			{
				billboardID = stoul(value);
			}
			else if (key == "billboard enabled")
			{
				isBillboardEnabled = stoi(value);
			}
		}

		//
		// LOAD SPOTLIGHT
		//

		ConsoleManager::WriteConsoleMessage(
			Caller::FILE,
			Type::DEBUG,
			"Loading spotlight '" + name + "' with ID '" + to_string(ID) + "' for scene '" + path(Engine::scenePath).parent_path().stem().string() + "'.\n");

		SpotLight::InitializeSpotLight(
			pos,
			rot,
			scale,
			txtFile,
			diffuse,
			intensity,
			distance,
			innerAngle,
			outerAngle,
			name,
			ID,
			isEnabled,
			billboardID,
			isBillboardEnabled);

		GameObject::nextID = ID + 1;
	}

	void GameObjectFile::LoadDirectionalLight(const string& file)
	{
		//
		// READ FROM DIRECTIONAL LIGHT FILE
		//

		ifstream directionalLightFile(file);
		if (!directionalLightFile.is_open())
		{
			ConsoleManager::WriteConsoleMessage(
				Caller::FILE,
				Type::EXCEPTION,
				"Error: Failed to open directional light file '" + file + "'!\n\n");
			return;
		}

		unordered_map<string, string> data;
		string line;
		while (getline(directionalLightFile, line))
		{
			if (!line.empty()
				&& line.find("=") != string::npos)
			{
				vector<string> splitLine = String::Split(line, '=');
				string key = splitLine[0];
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

				if (key == "name"
					|| key == "id"
					|| key == "enabled"
					|| key == "type"
					|| key == "position"
					|| key == "rotation"
					|| key == "scale"

					|| key == "txtFile"

					|| key == "diffuse"
					|| key == "intensity"
					|| key == "distance"

					|| key == "billboard id"
					|| key == "billboard enabled")
				{
					data[key] = value;
				}
			}
		}

		directionalLightFile.close();

		//
		// ASSIGN DIRECTIONAL LIGHT DATA TO VARIABLES
		//

		string name{};
		unsigned int ID{};
		bool isEnabled{};
		MeshComponent::MeshType type{};
		vec3 pos{};
		vec3 rot{};
		vec3 scale{};

		string txtFile{};

		vec3 diffuse{};
		float intensity{};

		unsigned int billboardID{};
		bool isBillboardEnabled{};

		for (const auto& [key, value] : data)
		{
			if (key == "name")
			{
				name = value;
			}
			else if (key == "id")
			{
				ID = stoul(value);
			}
			else if (key == "enabled")
			{
				isEnabled = stoi(value);
			}
			else if (key == "type")
			{
				auto typeAuto = magic_enum::enum_cast<MeshComponent::MeshType>(value);
				if (typeAuto.has_value())
				{
					type = typeAuto.value();
				}
			}
			else if (key == "position")
			{
				vector<string> split = String::Split(value, ',');
				vec3 newPos = vec3(stof(split[0]), stof(split[1]), stof(split[2]));
				pos = newPos;
			}
			else if (key == "rotation")
			{
				vector<string> split = String::Split(value, ',');
				vec3 newRot = vec3(stof(split[0]), stof(split[1]), stof(split[2]));
				rot = newRot;
			}
			else if (key == "scale")
			{
				vector<string> split = String::Split(value, ',');
				vec3 newScale = vec3(stof(split[0]), stof(split[1]), stof(split[2]));
				scale = newScale;
			}

			else if (key == "txtFile")
			{
				txtFile = value;
			}

			else if (key == "diffuse")
			{
				vector<string> split = String::Split(value, ',');
				vec3 newDiffuse = vec3(stof(split[0]), stof(split[1]), stof(split[2]));
				diffuse = newDiffuse;
			}
			else if (key == "intensity") intensity = stof(value);

			else if (key == "billboard id")
			{
				billboardID = stoul(value);
			}
			else if (key == "billboard enabled")
			{
				isBillboardEnabled = stoi(value);
			}
		}

		//
		// LOAD DIRECTIONAL LIGHT
		//

		ConsoleManager::WriteConsoleMessage(
			Caller::FILE,
			Type::DEBUG,
			"Loading directional light '" + name + "' with ID '" + to_string(ID) + "' for scene '" + path(Engine::scenePath).parent_path().stem().string() + "'.\n");

		DirectionalLight::InitializeDirectionalLight(
			pos,
			rot,
			scale,
			txtFile,
			diffuse,
			intensity,
			name,
			ID,
			isEnabled,
			billboardID,
			isBillboardEnabled);

		GameObject::nextID = ID + 1;
	}
}
