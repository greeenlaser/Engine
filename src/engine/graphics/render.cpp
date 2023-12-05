//external
#include "glad.h"

//engine
#include "console.h"
#include "render.h"
#include "shutdown.h"

namespace Graphics
{
	int RenderManager::WindowSetup()
	{
		Core::ConsoleManager::WriteConsoleMessage(
			Core::ConsoleManager::Caller::GLFW,
			Core::ConsoleManager::Type::INFO,
			"Initializing GLFW...\n");
		glfwInit();
		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
		glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
		Core::ConsoleManager::WriteConsoleMessage(
			Core::ConsoleManager::Caller::GLFW,
			Core::ConsoleManager::Type::SUCCESS,
			"GLFW initialized successfully!\n\n");
		Core::ConsoleManager::WriteConsoleMessage(
			Core::ConsoleManager::Caller::WINDOW_SETUP,
			Core::ConsoleManager::Type::INFO,
			"Creating window...\n");
		//create a window object holding all the windowing data
		RenderManager::window = glfwCreateWindow(
			RenderManager::SCR_WIDTH,
			RenderManager::SCR_HEIGHT,
			"Elypso engine",
			NULL,
			NULL);
		if (Graphics::RenderManager::window == NULL)
		{
			Core::ConsoleManager::WriteConsoleMessage(
				Core::ConsoleManager::Caller::GLFW,
				Core::ConsoleManager::Type::ERROR,
				"Error: Failed to create GLFW window!\n\n");
			Core::ShutdownManager::Shutdown();
			return -1;
		}
		glfwMakeContextCurrent(RenderManager::window);
		glfwSetFramebufferSizeCallback(
			RenderManager::window,
			RenderManager::UpdateAfterRescale);
		Core::ConsoleManager::WriteConsoleMessage(
			Core::ConsoleManager::Caller::WINDOW_SETUP,
			Core::ConsoleManager::Type::SUCCESS,
			"Window initialized successfully!\n\n");
		Core::ConsoleManager::WriteConsoleMessage(
			Core::ConsoleManager::Caller::GLAD,
			Core::ConsoleManager::Type::INFO,
			"Initializing GLAD...\n");
		//check if glad is initialized before continuing
		if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
		{
			Core::ConsoleManager::WriteConsoleMessage(
				Core::ConsoleManager::Caller::GLAD,
				Core::ConsoleManager::Type::ERROR,
				"Error: Failed to initialize GLAD!\n\n");
			return -1;
		}
		Core::ConsoleManager::WriteConsoleMessage(
			Core::ConsoleManager::Caller::GLAD,
			Core::ConsoleManager::Type::SUCCESS,
			"GLAD initialized successfully!\n\n");
		return 0;
	}
	//make sure the viewport matches the new window dimentions
	void RenderManager::UpdateAfterRescale(GLFWwindow* window, int width, int height)
	{
		glViewport(0, 0, width, height);
	}
	//this is run while the window is open
	void RenderManager::WindowLoop()
	{
		//clear the background to dark green
		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
		//clear color
		glClear(GL_COLOR_BUFFER_BIT);

		//render the triangle
		glBindVertexArray(ShaderManager::VAO);
		glDrawArrays(GL_TRIANGLES, 0, 3);

		//swap the front and back buffers
		glfwSwapBuffers(RenderManager::window);
		//poll for events and process them
		glfwPollEvents();
	}
}