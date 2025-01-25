//Copyright(C) 2025 Lost Empire Entertainment
//This program comes with ABSOLUTELY NO WARRANTY.
//This is free software, and you are welcome to redistribute it under certain conditions.
//Read LICENSE.md for more information.

#include <iostream>
#include <string>
#include <filesystem>

//external
#include "glad.h"
#include "quaternion.hpp"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "magic_enum.hpp"

//engine
#include "console.hpp"
#include "gui_engine.hpp"
#include "render.hpp"
#include "timeManager.hpp"
#include "stringUtils.hpp"
#include "input.hpp"
#include "selectobject.hpp"
#include "gameobject.hpp"
#include "sceneFile.hpp"
#include "configFile.hpp"
#include "importer.hpp"
#include "pointlight.hpp"
#include "spotlight.hpp"
#include "fileUtils.hpp"
#include "camera.hpp"
#include "gui_console.hpp"
#include "meshcomponent.hpp"
#include "materialcomponent.hpp"
#include "lightcomponent.hpp"
#include "audioplayercomponent.hpp"
#include "crane.hpp"
#if ENGINE_MODE
#include "compile.hpp"
#endif

using std::cout;
using std::endl;
using glm::radians;
using glm::lookAt;
using glm::quat;
using glm::rotate;
using glm::mat4;
using glm::length;
using std::ostringstream;
using std::fixed;
using std::setprecision;
using std::to_string;
using std::numeric_limits;
using std::filesystem::exists;
using std::filesystem::directory_iterator;
using std::filesystem::path;

using Graphics::Render;
using Core::ConsoleManager;
using EngineFile::SceneFile;
using EngineFile::ConfigFile;
using Graphics::Shape::GameObjectManager;
using Caller = Core::ConsoleManager::Caller;
using Type = Core::ConsoleManager::Type;
using Graphics::Shape::Importer;
using Graphics::Shape::PointLight;
using Graphics::Shape::SpotLight;
using Utils::File;
using Core::Input;
using Utils::String;
using Core::TimeManager;
using Graphics::Camera;
using Graphics::GUI::GUIConsole;
using Graphics::Components::MeshComponent;
using Graphics::Components::MaterialComponent;
using Graphics::Components::LightComponent;
using Graphics::Components::AudioPlayerComponent;
using Game::Crane;
#if ENGINE_MODE
using Core::Compilation;
#endif

namespace Core
{
    float increment;
    static bool startedHolding;

    void Input::ProcessKeyboardInput(GLFWwindow* window)
    {
    }

    void Input::ScrollCallback(GLFWwindow* window, double xoffset, double yoffset)
    {
#if ENGINE_MODE
        if (!Compilation::renderBuildingWindow
            && !ImGui::GetIO().WantCaptureMouse)
        {
            float combinedOffset = increment * static_cast<float>(yoffset);

            if (!Render::camera.cameraEnabled)
            {
                if (Select::selectedObj != nullptr)
                {
                    if (objectAction == ObjectAction::move)
                    {
                        vec3 pos = Select::selectedObj->GetTransform()->GetPosition();
                        if (axis == "X") pos = vec3(pos.x + combinedOffset, pos.y, pos.z);
                        else if (axis == "Y") pos = vec3(pos.x, pos.y + combinedOffset, pos.z);
                        else if (axis == "Z") pos = vec3(pos.x, pos.y, pos.z + combinedOffset);

                        Select::selectedObj->GetTransform()->SetPosition(pos);
                        if (!SceneFile::unsavedChanges) Render::SetWindowNameAsUnsaved(true);
                    }
                    else if (objectAction == ObjectAction::rotate)
                    {
                        vec3 rot = Select::selectedObj->GetTransform()->GetRotation();
                        if (axis == "X") rot = vec3(rot.x + combinedOffset * 10, rot.y, rot.z);
                        else if (axis == "Y") rot = vec3(rot.x, rot.y + combinedOffset * 10, rot.z);
                        else if (axis == "Z") rot = vec3(rot.x, rot.y, rot.z + combinedOffset * 10);
                        Select::selectedObj->GetTransform()->SetRotation(rot);
                        if (!SceneFile::unsavedChanges) Render::SetWindowNameAsUnsaved(true);
                    }
                    else if (objectAction == ObjectAction::scale)
                    {
                        vec3 scale = Select::selectedObj->GetTransform()->GetScale();
                        if (axis == "X") scale = vec3(scale.x + combinedOffset, scale.y, scale.z);
                        else if (axis == "Y") scale = vec3(scale.x, scale.y + combinedOffset, scale.z);
                        else if (axis == "Z") scale = vec3(scale.x, scale.y, scale.z + combinedOffset);

                        Select::selectedObj->GetTransform()->SetScale(scale);
                        if (!SceneFile::unsavedChanges) Render::SetWindowNameAsUnsaved(true);
                    }
                }
            }
        }
#endif
    }

    void Input::KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
    {
#if ENGINE_MODE
        if (!Compilation::renderBuildingWindow
            && !Render::camera.cameraEnabled)
        {
            if (!ImGui::GetIO().WantCaptureMouse)
            {
                //copy selected object
                if (key == GLFW_KEY_C
                    && mods == GLFW_MOD_CONTROL
                    && action == GLFW_PRESS
                    && Select::selectedObj != nullptr
                    && Select::selectedObj->GetComponent<MeshComponent>()->GetMeshType()
                    != MeshComponent::MeshType::directional_light) //cannot copy directional light
                {
                    Copy();
                }

                //paste selected object
                if (key == GLFW_KEY_V
                    && mods == GLFW_MOD_CONTROL
                    && action == GLFW_PRESS
                    && copiedObject.size() != 0)
                {
                    Paste();
                }

                //delete selected gameobject
                if (key == GLFW_KEY_DELETE
                    && action == GLFW_PRESS
                    && Select::selectedObj != nullptr)
                {
                    shared_ptr<GameObject> selectedObj = Select::selectedObj;
                    GameObjectManager::DestroyGameObject(selectedObj, false);
                }

                if (Select::selectedObj != nullptr)
                {
                    //switch to X axis
                    if (key == GLFW_KEY_X
                        && action == GLFW_PRESS
                        && axis != "X")
                    {
                        axis = "X";
                    }
                    //switch to Y axis
                    if (key == GLFW_KEY_Y
                        && action == GLFW_PRESS
                        && axis != "Y")
                    {
                        axis = "Y";
                    }
                    //switch to Z axis
                    if (key == GLFW_KEY_Z
                        && action == GLFW_PRESS
                        && axis != "Z")
                    {
                        axis = "Z";
                    }

                    //switch to move action
                    if (key == GLFW_KEY_W
                        && action == GLFW_PRESS
                        && objectAction != ObjectAction::move)
                    {
                        objectAction = ObjectAction::move;
                    }
                    //switch to rotate action
                    if (key == GLFW_KEY_E
                        && action == GLFW_PRESS
                        && objectAction != ObjectAction::rotate)
                    {
                        objectAction = ObjectAction::rotate;
                    }
                    //switch to scale action
                    if (key == GLFW_KEY_R
                        && action == GLFW_PRESS
                        && objectAction != ObjectAction::scale)
                    {
                        objectAction = ObjectAction::scale;
                    }
                }
            }

            //compile game
            if (key == GLFW_KEY_B
                && mods == GLFW_MOD_CONTROL
                && action == GLFW_PRESS)
            {
                Compilation::installerType = Compilation::InstallerType::compile;
                Compilation::Compile();
            }

            //clean rebuild
            if (key == GLFW_KEY_B
                && (mods == (GLFW_MOD_CONTROL | GLFW_MOD_SHIFT))
                && action == GLFW_PRESS)
            {
                Compilation::installerType = Compilation::InstallerType::reset;
                Compilation::Compile();
            }

            //run game
            if (key == GLFW_KEY_F5
                && action == GLFW_PRESS)
            {
                Compilation::Run();
            }
        }
#else
        if (!Render::camera.cameraEnabled)
        {
            //save current scene
            if (key == GLFW_KEY_S
                && mods == GLFW_MOD_CONTROL
                && action == GLFW_PRESS)
            {
                SceneFile::SaveScene();
                ConfigFile::SaveConfigFile();
            }

            //toggle game console on and off
            if (key == GLFW_KEY_PAGE_UP
                && action == GLFW_PRESS)
            {
                GUIConsole::renderConsole = !GUIConsole::renderConsole;
            }
        }
#endif

        //save current scene
        if (key == GLFW_KEY_S
            && mods == GLFW_MOD_CONTROL
            && action == GLFW_PRESS)
        {
            SceneFile::SaveScene();
            ConfigFile::SaveConfigFile();
        }

        //activate crane
        if (key == GLFW_KEY_E
            && action == GLFW_PRESS
            && !Crane::isCraneActivated
            && Crane::crane != nullptr
            && Crane::craneController != nullptr)
        {
            //calculate the distance between crane and camera
            vec3 cameraPos = Render::camera.GetCameraPosition();
            vec3 craneControllerPos = Crane::craneController->GetTransform()->GetPosition();
            float distance = length(cameraPos - craneControllerPos);

            //if distance is less than 1 then crane can be "entered"
            if (distance < 1.75f)
            {
                Crane::EnterCrane();

                cout << "!!!!!!!!!! activated crane!\n";
            }
            else
            {
                cout << "!!!!!!!!!! not in range of crane (" << to_string(distance) << ")!\n";
            }
        }

        //move crane front
        if (key == GLFW_KEY_W
            && action == GLFW_PRESS
            && Crane::isCraneActivated
            && !Crane::isCraneMovingY
            && !Crane::isCraneMovingXZ)
        {
            Crane::StartCraneMove(vec3(0.0f, 0.0f, 2.0f));
        }
        //move crane back
        if (key == GLFW_KEY_S
            && action == GLFW_PRESS
            && Crane::isCraneActivated
            && !Crane::isCraneMovingY
            && !Crane::isCraneMovingXZ)
        {
            Crane::StartCraneMove(vec3(0.0f, 0.0f, -2.0f));
        }
        //move crane left
        if (key == GLFW_KEY_A
            && action == GLFW_PRESS
            && Crane::isCraneActivated
            && !Crane::isCraneMovingY
            && !Crane::isCraneMovingXZ)
        {
            Crane::StartCraneMove(vec3(2.0f, 0.0f, 0.0f));
        }
        //move crane right
        if (key == GLFW_KEY_D
            && action == GLFW_PRESS
            && Crane::isCraneActivated
            && !Crane::isCraneMovingY
            && !Crane::isCraneMovingXZ)
        {
            Crane::StartCraneMove(vec3(-2.0f, 0.0f, 0.0f));
        }

        //grab barrel
        if (key == GLFW_KEY_SPACE
            && action == GLFW_PRESS
            && Crane::isCraneActivated
            && !Crane::isCraneMovingY
            && !Crane::isCraneMovingXZ)
        {
            Crane::StartBarrelGrab();
        }

        //deactivate crane
        if (key == GLFW_KEY_TAB
            && action == GLFW_PRESS
            && Crane::isCraneActivated)
        {
            Crane::ExitCrane();
            cout << "!!!!!!!!!! deactivated crane!\n";
        }
    }

    void Input::MouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
    {
#if ENGINE_MODE
#else
        if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS)
        {
            double x, y;
            glfwGetCursorPos(window, &x, &y);

            int renderSizeX, renderSizeY;
            glfwGetFramebufferSize(window, &renderSizeX, &renderSizeY);

            if (x >= 0
                && x <= renderSizeX
                && y >= 0
                && y <= renderSizeY)
            {
                Input::ObjectInteraction(
                    static_cast<float>(renderSizeX),
                    static_cast<float>(renderSizeY),
                    x,
                    y);
            }
        }
#endif
    }

    void Input::MouseMovementCallback(GLFWwindow* window, double xpos, double ypos)
    {}

    void Input::Copy()
    {
        copiedObject.clear();

        shared_ptr<GameObject> selectedObj = Select::selectedObj;
        copiedObject.clear();
        copiedObject["txtPath"] = selectedObj->GetTxtFilePath();
        copiedObject["name"] = selectedObj->GetName();
        copiedObject["id"] = to_string(selectedObj->GetID());
        copiedObject["pos"] =
            to_string(selectedObj->GetTransform()->GetPosition().x) + ","
            + to_string(selectedObj->GetTransform()->GetPosition().y) + ","
            + to_string(selectedObj->GetTransform()->GetPosition().z);
        copiedObject["rot"] =
            to_string(selectedObj->GetTransform()->GetRotation().x) + ","
            + to_string(selectedObj->GetTransform()->GetRotation().y) + ","
            + to_string(selectedObj->GetTransform()->GetRotation().z);
        copiedObject["scale"] =
            to_string(selectedObj->GetTransform()->GetScale().x) + ","
            + to_string(selectedObj->GetTransform()->GetScale().y) + ","
            + to_string(selectedObj->GetTransform()->GetScale().z);

        MeshComponent::MeshType type = selectedObj->GetComponent<MeshComponent>()->GetMeshType();
        if (type == MeshComponent::MeshType::model)
        {
            copiedObject["type"] = "model";

            string modelPath{};
            string modelFolder = (path(Engine::projectPath) / path(selectedObj->GetTxtFilePath()).parent_path().string()).string();
            for (const auto& file : directory_iterator(modelFolder))
            {
                string extension = path(file).extension().string();
                if (extension == ".fbx"
                    || extension == ".obj"
                    || extension == ".gltf")
                {
                    modelPath = path(file).string();
                    break;
                }
            }
            copiedObject["model"] = modelPath;

            copiedObject["diffuseTexture"] = selectedObj->GetComponent<MaterialComponent>()->GetTextureName(MaterialComponent::TextureType::diffuse);
            copiedObject["specularTexture"] = selectedObj->GetComponent<MaterialComponent>()->GetTextureName(MaterialComponent::TextureType::specular);
            copiedObject["normalTexture"] = selectedObj->GetComponent<MaterialComponent>()->GetTextureName(MaterialComponent::TextureType::normal);
            copiedObject["heightTexture"] = selectedObj->GetComponent<MaterialComponent>()->GetTextureName(MaterialComponent::TextureType::height);

            copiedObject["shininess"] = "32";
        }

        else if (type == MeshComponent::MeshType::point_light)
        {
            copiedObject["type"] = "point";

            copiedObject["diffuse"] =
                to_string(selectedObj->GetComponent<LightComponent>()->GetDiffuse().x) + ","
                + to_string(selectedObj->GetComponent<LightComponent>()->GetDiffuse().y) + ","
                + to_string(selectedObj->GetComponent<LightComponent>()->GetDiffuse().z);
            copiedObject["intensity"] = to_string(selectedObj->GetComponent<LightComponent>()->GetIntensity());
            copiedObject["distance"] = to_string(selectedObj->GetComponent<LightComponent>()->GetDistance());
        }

        else if (type == MeshComponent::MeshType::spot_light)
        {
            copiedObject["type"] = "spot";

            copiedObject["diffuse"] =
                to_string(selectedObj->GetComponent<LightComponent>()->GetDiffuse().x) + ","
                + to_string(selectedObj->GetComponent<LightComponent>()->GetDiffuse().y) + ","
                + to_string(selectedObj->GetComponent<LightComponent>()->GetDiffuse().z);
            copiedObject["intensity"] = to_string(selectedObj->GetComponent<LightComponent>()->GetIntensity());
            copiedObject["distance"] = to_string(selectedObj->GetComponent<LightComponent>()->GetDistance());
            copiedObject["innerAngle"] = to_string(selectedObj->GetComponent<LightComponent>()->GetInnerAngle());
            copiedObject["outerAngle"] = to_string(selectedObj->GetComponent<LightComponent>()->GetOuterAngle());
        }

        auto apc = selectedObj->GetComponent<AudioPlayerComponent>();
        if (apc)
        {
            string audioFileName = apc->GetName();
            bool is3D = apc->Is3D();
            float currVolume = apc->GetVolume();

            copiedObject["audioFileName"] = audioFileName;
            copiedObject["is3D"] = to_string(is3D);
            copiedObject["currentVolume"] = to_string(currVolume);
        }

        selectedObj = nullptr;

        ConsoleManager::WriteConsoleMessage(
            Caller::FILE,
            Type::INFO,
            "Successfully copied gameobject '" + copiedObject["name"] + "'!\n");
    }

    void Input::Paste()
    {
        string name = copiedObject["name"];
        unsigned int nextID = ++GameObject::nextID;
        unsigned int nextID2 = ++GameObject::nextID;

        vector<string> posSplit = String::Split(copiedObject["pos"].c_str(), ',');
        vec3 newPos = vec3(stof(posSplit[0]), stof(posSplit[1]), stof(posSplit[2]));
        /*
        vec3 newPos = Render::camera.GetCameraPosition() + Render::camera.GetFront() * 5.0f;
        int resultX = static_cast<int>(newPos.x);
        int resultY = static_cast<int>(newPos.y);
        int resultZ = static_cast<int>(newPos.z);
        newPos = vec3(resultX, resultY, resultZ);
        */

        vector<string> rotSplit = String::Split(copiedObject["rot"].c_str(), ',');
        vec3 rot = vec3(stof(rotSplit[0]), stof(rotSplit[1]), stof(rotSplit[2]));

        vector<string> scaleSplit = String::Split(copiedObject["scale"].c_str(), ',');
        vec3 scale = vec3(stof(scaleSplit[0]), stof(scaleSplit[1]), stof(scaleSplit[2]));

        //if a regular model was pasted
        if (copiedObject["type"] == "model")
        {
            string extension;
            string originPath = (path(Engine::currentGameobjectsPath) / name).string();

            for (const auto& entry : directory_iterator(originPath))
            {
                extension = path(entry).extension().string();
                if (extension == ".fbx"
                    || extension == ".obj"
                    || extension == ".glfw")
                {
                    originPath += "\\" + name + extension;
                    break;
                }
            }
            string targetPath = File::AddIndex(Engine::currentGameobjectsPath, name);
            string targetName = path(targetPath).stem().string();
            string targetNameAndExtension = targetName + extension;

            File::CreateNewFolder(targetPath);
            string destinationPath = (path(targetPath) / targetNameAndExtension).string();
            File::CopyFileOrFolder(originPath, destinationPath);
            
            string diffTexturePath = copiedObject["diffuseTexture"];
            if (diffTexturePath != "DEFAULTDIFF")
            {
                string diffTextureFile = path(diffTexturePath).filename().string();
                string diffDestinationPath = (path(targetPath) / diffTextureFile).string();

                File::CopyFileOrFolder(diffTexturePath, diffDestinationPath);
            }

            string specTexturePath = copiedObject["specularTexture"];
            if (specTexturePath != "DEFAULTSPEC")
            {
                string specTextureFile = path(specTexturePath).filename().string();
                string specDestinationPath = (path(targetPath) / specTextureFile).string();

                File::CopyFileOrFolder(specTexturePath, specDestinationPath);
            }

            Importer::Initialize(
                newPos,
                rot,
                scale,
                copiedObject["model"],
                diffTexturePath,
                specTexturePath,
                copiedObject["normalTexture"],
                copiedObject["heightTexture"],
                targetName,
                nextID,
                true);

            if (!SceneFile::unsavedChanges) Render::SetWindowNameAsUnsaved(true);
        }

        //if a point light was copied
        else if (copiedObject["type"] == "point")
        {
            string targetPath = File::AddIndex(Engine::currentGameobjectsPath, name);
            string targetName = path(targetPath).stem().string();
            File::CreateNewFolder(targetPath);

            string targetNameAndExtension = targetName + ".txt";
            string filePath = (path(targetPath) / targetNameAndExtension).string();

            vector<string> diffSplit = String::Split(copiedObject["diffuse"].c_str(), ',');
            vec3 diff = vec3(stof(diffSplit[0]), stof(diffSplit[1]), stof(diffSplit[2]));

            shared_ptr<GameObject> newPointLight = PointLight::InitializePointLight(
                newPos,
                rot,
                scale,
                filePath,
                diff,
                stof(copiedObject["intensity"]),
                stof(copiedObject["distance"]),
                targetName,
                nextID,
                true,
                nextID2,
                true);

            if (!SceneFile::unsavedChanges) Render::SetWindowNameAsUnsaved(true);
        }

        //if a spotlight was copied
        else if (copiedObject["type"] == "spot")
        {
            string targetPath = File::AddIndex(Engine::currentGameobjectsPath, name);
            string targetName = path(targetPath).stem().string();
            string targetNameAndExtension = targetName + ".txt";
            File::CreateNewFolder(targetPath);

            string filePath = (path(targetPath) / targetNameAndExtension).string();

            vector<string> diffSplit = String::Split(copiedObject["diffuse"].c_str(), ',');
            vec3 diff = vec3(stof(diffSplit[0]), stof(diffSplit[1]), stof(diffSplit[2]));

            shared_ptr<GameObject> newSpotlight = SpotLight::InitializeSpotLight(
                newPos,
                rot,
                scale,
                filePath,
                diff,
                stof(copiedObject["intensity"]),
                stof(copiedObject["distance"]),
                stof(copiedObject["innerAngle"]),
                stof(copiedObject["outerAngle"]),
                targetName,
                nextID,
                true,
                nextID2,
                true);

            if (!SceneFile::unsavedChanges) Render::SetWindowNameAsUnsaved(true);
        }

        if (copiedObject.find("audioFileName") != copiedObject.end())
        {
            string audioFileName = copiedObject["audioFileName"];
            bool is3D = stoi(copiedObject["is3D"]);
            float currentVolume = stof(copiedObject["currentVolume"]);

            Select::selectedObj->AddComponent<AudioPlayerComponent>();
            auto apc = Select::selectedObj->GetComponent<AudioPlayerComponent>();
            apc->SetName(audioFileName);
            apc->Set3DState(is3D);
            apc->SetVolume(currentVolume);
        }

        ConsoleManager::WriteConsoleMessage(
            Caller::FILE,
            Type::INFO,
            "Successfully pasted gameobject '" + copiedObject["name"] + "'!\n");

        copiedObject.clear();
    }

    void Input::LimitMovement()
    {
        bool limitCamera = stoi(ConfigFile::GetValue("limit_camera", true));
        if (limitCamera)
        {
            float camXLimit = stof(ConfigFile::GetValue("camXLimit", true));
            float camYLimit = stof(ConfigFile::GetValue("camYLimit", true));
            float camZLimit = stof(ConfigFile::GetValue("camZLimit", true));

            vec3 camPos = Render::camera.GetCameraPosition();
            if (camPos.y != camYLimit) camPos.y = camYLimit;

            if (camPos.x > camXLimit) camPos.x = camXLimit;
            if (camPos.x < -camXLimit) camPos.x = -camXLimit;
            
            if (camPos.z > camZLimit) camPos.z = camZLimit;
            if (camPos.z < -camZLimit) camPos.z = -camZLimit;

            Render::camera.SetCameraPosition(camPos);
        }
    }

    void Input::SceneWindowInput()
    {
        DragCamera();
        MoveCamera();
        SetCameraSpeed();
    }

    void Input::ObjectInteraction(float width, float height, double posX, double posY)
    {
#if ENGINE_MODE
        if (!Compilation::renderBuildingWindow
            && !Render::camera.cameraEnabled)
        {
            Select::Ray ray = Select::RayFromMouse(
                width,
                height,
                posX,
                posY,
                Render::view,
                Render::projection);

            vector<shared_ptr<GameObject>> objects = GameObjectManager::GetObjects();
            int index = Select::CheckRayObjectIntersections(ray, objects);

            //if user did not press any valid gameobject
            if (index == -1)
            {
                Select::isObjectSelected = false;
                Select::selectedObj = nullptr;

                objectAction = ObjectAction::none;
                axis = "";
            }
            else
            {
                Select::selectedObj = objects[index];
                Select::isObjectSelected = true;

                if (objectAction == ObjectAction::none)
                {
                    objectAction = ObjectAction::move;
                }
                if (axis == "") axis = "X";
            }
        }
#endif
    }

    void Input::DragCamera()
    {
        if (glfwRawMouseMotionSupported())
        {
            glfwSetInputMode(Render::window, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);
        }
#if ENGINE_MODE
        if (glfwGetMouseButton(Render::window, GLFW_MOUSE_BUTTON_RIGHT)) 
        {
            glfwSetInputMode(Render::window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        }
        else 
        {
            glfwSetInputMode(Render::window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        }
#else
        if (glfwGetWindowAttrib(Render::window, GLFW_FOCUSED))
        {
            glfwSetInputMode(Render::window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        }
        else
        {
            glfwSetInputMode(Render::window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        }
#endif

#if ENGINE_MODE
        Render::camera.cameraEnabled = ImGui::IsMouseDown(ImGuiMouseButton_Right);

        if (ImGui::IsMouseDragging(ImGuiMouseButton_Right))
        {
            ImVec2 mouseDelta = ImGui::GetIO().MouseDelta;
            double mouseDeltaX = mouseDelta.x;
            double mouseDeltaY = mouseDelta.y;

            //camera rotation with mouse drag
            if (mouseDeltaX != 0.0f
                || mouseDeltaY != 0.0f)
            {
                float sensitivity = 1.0f;

                //invert Y-axis of mouse delta to work with inverted imgui scene window
                mouseDeltaY = -mouseDeltaY;

                Render::camera.RotateCamera(
                    mouseDeltaX * sensitivity,
                    mouseDeltaY * sensitivity);

                if (!SceneFile::unsavedChanges) Render::SetWindowNameAsUnsaved(true);
            }
        }
#else
        static double lastX = 0.0, lastY = 0.0;
        static bool firstMove = true;

        double currentX, currentY;
        glfwGetCursorPos(Render::window, &currentX, &currentY);

        Render::camera.cameraEnabled =
            glfwGetMouseButton(Render::window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS;

        if (Render::camera.cameraEnabled)
        {
            if (firstMove)
            {
                lastX = currentX;
                lastY = currentY;
                firstMove = false;
            }

            double mouseDeltaX = currentX - lastX;
            double mouseDeltaY = currentY - lastY;

            lastX = currentX;
            lastY = currentY;

            //camera rotation with mouse drag
            if (mouseDeltaX != 0.0f
                || mouseDeltaY != 0.0f)
            {
                float sensitivity = 1.0f;

                //invert Y-axis of mouse delta to work with inverted imgui scene window
                mouseDeltaY = -mouseDeltaY;

                Render::camera.RotateCamera(
                    mouseDeltaX * sensitivity,
                    mouseDeltaY * sensitivity);

                if (!SceneFile::unsavedChanges) Render::SetWindowNameAsUnsaved(true);
            }
        }
        else firstMove = true;
#endif
    }

    void Input::MoveCamera()
    {
#if ENGINE_MODE
        if (Render::camera.cameraEnabled
            && !startedHolding)
        {
            startedHolding = true;
            appliedUpdate = false;
        }
        if (!Render::camera.cameraEnabled
            && (startedHolding
                || (!startedHolding
                    && Camera::lastKnownRotation == vec3(0)
                    && Render::camera.GetCameraRotation() != vec3(0))))
        {
            Camera::lastKnownRotation = Render::camera.GetCameraRotation();
            startedHolding = false;
        }
#else
        Render::camera.cameraEnabled = true;
#endif

        if (Render::camera.cameraEnabled)
        {
            float moveSpeedMultiplier = stof(ConfigFile::GetValue("camera_speedMultiplier"));

            bool isLeftShiftPressed;
#if ENGINE_MODE
            isLeftShiftPressed = (ImGui::IsKeyDown(ImGuiKey_LeftShift));
#else
            isLeftShiftPressed = (glfwGetKey(Render::window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS);
#endif
            float currentSpeed = isLeftShiftPressed
                ? static_cast<float>(2.0f * moveSpeedMultiplier * TimeManager::deltaTime)
                : static_cast<float>(1.0f * moveSpeedMultiplier * TimeManager::deltaTime);

            vec3 front = Render::camera.GetFront();
            vec3 right = Render::camera.GetRight();

            //camera forwards
#if ENGINE_MODE
            if (ImGui::IsKeyDown(ImGuiKey_W))
#else
            if (glfwGetKey(Render::window, GLFW_KEY_W) == GLFW_PRESS)
#endif
            {
                if (!Crane::isCraneActivated)
                {
                    Render::camera.SetCameraPosition(
                        Render::camera.GetCameraPosition()
                        + Render::camera.cameraSpeed * currentSpeed * front);
                }

                if (!SceneFile::unsavedChanges) Render::SetWindowNameAsUnsaved(true);
            }
            //camera backwards
#if ENGINE_MODE
            if (ImGui::IsKeyDown(ImGuiKey_S))
#else
            if (glfwGetKey(Render::window, GLFW_KEY_S) == GLFW_PRESS)
#endif
            {
                if (!Crane::isCraneActivated)
                {
                    Render::camera.SetCameraPosition(
                        Render::camera.GetCameraPosition()
                        - Render::camera.cameraSpeed * currentSpeed * front);

                    if (!SceneFile::unsavedChanges) Render::SetWindowNameAsUnsaved(true);
                }
            }
            //camera left
#if ENGINE_MODE
            if (ImGui::IsKeyDown(ImGuiKey_A))
#else
            if (glfwGetKey(Render::window, GLFW_KEY_A) == GLFW_PRESS)
#endif
            {
                if (!Crane::isCraneActivated)
                {
                    Render::camera.SetCameraPosition(
                        Render::camera.GetCameraPosition()
                        - Render::camera.cameraSpeed * currentSpeed * right);

                    if (!SceneFile::unsavedChanges) Render::SetWindowNameAsUnsaved(true);
                }
            }
            //camera right
#if ENGINE_MODE
            if (ImGui::IsKeyDown(ImGuiKey_D))
#else
            if (glfwGetKey(Render::window, GLFW_KEY_D) == GLFW_PRESS)
#endif
            {
                if (!Crane::isCraneActivated)
                {
                    Render::camera.SetCameraPosition(
                        Render::camera.GetCameraPosition()
                        + Render::camera.cameraSpeed * currentSpeed * right);

                    if (!SceneFile::unsavedChanges) Render::SetWindowNameAsUnsaved(true);
                }
            }
            //camera up
#if ENGINE_MODE
            if (ImGui::IsKeyDown(ImGuiKey_E))
#else
            if (glfwGetKey(Render::window, GLFW_KEY_E) == GLFW_PRESS)
#endif
            {
                if (!Crane::isCraneActivated)
                {
                    Render::camera.SetCameraPosition(
                        Render::camera.GetCameraPosition()
                        + Render::camera.GetUp() * Render::camera.cameraSpeed * currentSpeed);

                    if (!SceneFile::unsavedChanges) Render::SetWindowNameAsUnsaved(true);
                }
            }
            //camera down
#if ENGINE_MODE
            if (ImGui::IsKeyDown(ImGuiKey_Q))
#else
            if (glfwGetKey(Render::window, GLFW_KEY_Q) == GLFW_PRESS)
#endif
            {
                Render::camera.SetCameraPosition(
                    Render::camera.GetCameraPosition()
                    - Render::camera.GetUp() * Render::camera.cameraSpeed * currentSpeed);

                if (!SceneFile::unsavedChanges) Render::SetWindowNameAsUnsaved(true);
            }
        }
        else increment = ImGui::IsKeyDown(ImGuiKey_LeftShift) ? 1.0f : 0.1f;
    }

    void Input::SetCameraSpeed()
    {
        if (Render::camera.cameraEnabled)
        {
            float yoffset = ImGui::GetIO().MouseWheel;
            float combinedOffset = increment * static_cast<float>(yoffset);
            float currentSpeed = stof(ConfigFile::GetValue("camera_speedMultiplier"));
            float newSpeed = currentSpeed + currentSpeed * combinedOffset;

            if (newSpeed > 100.0f) newSpeed = 100.0f;
            if (newSpeed < 0.1f) newSpeed = 0.1f;

            ConfigFile::SetValue("camera_speedMultiplier", to_string(newSpeed));
        }
    }
}