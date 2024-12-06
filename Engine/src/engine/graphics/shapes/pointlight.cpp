//Copyright(C) 2024 Lost Empire Entertainment
//This program comes with ABSOLUTELY NO WARRANTY.
//This is free software, and you are welcome to redistribute it under certain conditions.
//Read LICENSE.md for more information.

//external
#include "quaternion.hpp"
#include "matrix_transform.hpp"

//engine
#include "pointlight.hpp"
#include "render.hpp"
#include "selectobject.hpp"
#include "billboard.hpp"
#include "console.hpp"
#include "meshcomponent.hpp"
#include "lighttcomponent.hpp"
#if ENGINE_MODE
#include "gui_scenewindow.hpp"
#endif

using glm::translate;
using glm::quat;

using Graphics::Shader;
using Graphics::Components::Mesh;
using MeshType = Graphics::Components::Mesh::MeshType;
using Graphics::Render;
using Core::Select;
using Core::ConsoleManager;
using Caller = Core::ConsoleManager::Caller;
using Type = Core::ConsoleManager::Type;
using Graphics::Components::PointLightComponent;
using Graphics::Components::LightComponent;
#if ENGINE_MODE
using Graphics::GUI::GUISceneWindow;
#endif

namespace Graphics::Shape
{
	shared_ptr<GameObject> PointLight::InitializePointLight(
		const vec3& pos,
		const vec3& rot,
		const vec3& scale,
		const string& txtFilePath,
		const string& vertShader,
		const string& fragShader,
		const vec3& diffuse,
		const float& intensity,
		const float& distance,
		string& name,
		unsigned int& id,
		const bool& isEnabled,
		const bool& isMeshEnabled,

		const string& billboardVertShader,
		const string& billboardFragShader,
		const string& billboardDiffTexture,
		const float& billboardShininess,
		string& billboardName,
		unsigned int& billboardID,
		const bool& isBillboardEnabled)
	{
		cout << "vert shader: " << vertShader << "\n";
		cout << "frag shader: " << fragShader << "\n";
		cout << "txt file path: " << txtFilePath << "\n";

		auto obj = GameObject::Create(
			name, 
			id, 
			isEnabled);

		float vertices[] =
		{
			//edges of the cube
			-0.5f, -0.5f, -0.5f,
			 0.5f, -0.5f, -0.5f,

			 0.5f, -0.5f, -0.5f,
			 0.5f,  0.5f, -0.5f,

			 0.5f,  0.5f, -0.5f,
			-0.5f,  0.5f, -0.5f,

			-0.5f,  0.5f, -0.5f,
			-0.5f, -0.5f, -0.5f,

			-0.5f, -0.5f,  0.5f,
			 0.5f, -0.5f,  0.5f,

			 0.5f, -0.5f,  0.5f,
			 0.5f,  0.5f,  0.5f,

			 0.5f,  0.5f,  0.5f,
			-0.5f,  0.5f,  0.5f,

			-0.5f,  0.5f,  0.5f,
			-0.5f, -0.5f,  0.5f,

			//connecting edges
			-0.5f, -0.5f, -0.5f,
			-0.5f, -0.5f,  0.5f,

			 0.5f, -0.5f, -0.5f,
			 0.5f, -0.5f,  0.5f,

			 0.5f,  0.5f, -0.5f,
			 0.5f,  0.5f,  0.5f,

			-0.5f,  0.5f, -0.5f,
			-0.5f,  0.5f,  0.5f,
		};

		auto lightConfig = LightComponent::LightConfig
		{
			diffuse,
			intensity,
			vertShader,
			fragShader,
			isMeshEnabled,
			billboardVertShader,
			billboardFragShader,
			billboardDiffTexture,
			billboardShininess,
			isBillboardEnabled
		};

		auto pointLight = obj->AddComponent<PointLightComponent>
		(
			lightConfig,
			distance 
		);

		pointLight->Initialize(obj, vertices, pos, rot, scale);

		obj->SetTxtFilePath(txtFilePath);

		GameObjectManager::AddGameObject(obj);
		GameObjectManager::AddOpaqueObject(obj);

#if ENGINE_MODE
		GUISceneWindow::UpdateCounts();
#endif
		Select::selectedObj = obj;
		Select::isObjectSelected = true;

		ConsoleManager::WriteConsoleMessage(
			Caller::FILE,
			Type::DEBUG,
			"Successfully initialized " + obj->GetName() + " with ID " + to_string(obj->GetID()) + "\n");

		return obj;
	}
}