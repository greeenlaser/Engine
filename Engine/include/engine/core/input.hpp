//Copyright(C) 2025 Lost Empire Entertainment
//This program comes with ABSOLUTELY NO WARRANTY.
//This is free software, and you are welcome to redistribute it under certain conditions.
//Read LICENSE.md for more information.

#pragma once

#include <map>
#include <string>

//external
#include "glfw3.h"

//engine
#include "input.hpp"

namespace Core
{
    using std::map;
    using std::string;

    class Input
    {
    public:
        static inline bool appliedUpdate;

        static inline string axis = "";
        enum class ObjectAction
        {
            none,
            move,
            rotate,
            scale
        };
        static inline ObjectAction objectAction = ObjectAction::none;

        static void ProcessKeyboardInput(GLFWwindow* window);

        static void ScrollCallback(GLFWwindow* window, double xoffset, double yoffset);
        static void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
        static void MouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
        static void MouseMovementCallback(GLFWwindow* window, double xpos, double ypos);

        static inline map<string, string> copiedObject;
        static void Copy();
        static void Paste();

        /// <summary>
        /// Hard-coded limit to camera Y height and max positions inside room where game takes place inside of.
        /// </summary>
        static void LimitMovement();

        static void SceneWindowInput();
        static void ObjectInteraction(float width, float height, double posX, double posY);
        static void DragCamera();
        static void MoveCamera();
        static void SetCameraSpeed();
    };
}