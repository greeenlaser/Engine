//Copyright(C) 2025 Lost Empire Entertainment
//This program comes with ABSOLUTELY NO WARRANTY.
//This is free software, and you are welcome to redistribute it under certain conditions.
//Read LICENSE.md for more information.

#include <iostream>
#include <map>
#include <fstream>
#include <string>

//external
#include "glad.h"
#include "quaternion.hpp"
#include "matrix_transform.hpp"

//engine
#include "render.hpp"
#include "model.hpp"
#include "shader.hpp"
#include "texture.hpp"
#include "core.hpp"
#include "console.hpp"
#include "stringUtils.hpp"
#include "selectobject.hpp"
#include "gameObjectFile.hpp"
#include "transformcomponent.hpp"
#include "meshcomponent.hpp"
#include "materialcomponent.hpp"
#include "lightcomponent.hpp"
#if ENGINE_MODE
#include "gui_scenewindow.hpp"
#endif

using std::cout;
using std::endl;
using std::to_string;
using std::map;
using glm::translate;
using glm::rotate;
using glm::radians;
using glm::quat;
using glm::scale;
using std::ofstream;
using std::ifstream;
using std::filesystem::exists;
using std::stoul;
using std::stof;

using Graphics::Render;
using Graphics::Shader;
using Graphics::Texture;
using Graphics::Components::TransformComponent;
using Graphics::Components::MeshComponent;
using Graphics::Components::MaterialComponent;
using Graphics::Components::LightComponent;
using Graphics::Components::AssimpVertex;
using MeshType = Graphics::Components::MeshComponent::MeshType;
using Core::Engine;
using Core::ConsoleManager;
using Caller = Core::ConsoleManager::Caller;
using Type = Core::ConsoleManager::Type;
using Utils::String;
using Core::Select;
using EngineFile::GameObjectFile;
#if ENGINE_MODE
using Graphics::GUI::GUISceneWindow;
#endif

namespace Graphics::Shape
{
	shared_ptr<GameObject> Model::Initialize(
		const vec3& pos,
		const vec3& rot,
		const vec3& scale,
		const string& txtFilePath,
		const string& modelPath,
		const string& diffTexture,
		const string& specTexture,
		const string& normalTexture,
		const string& heightTexture,
		const vector<AssimpVertex> vertices,
		const vector<unsigned int> indices,
		string& name,
		unsigned int& id,
		const bool& isEnabled)
	{
		auto obj = make_shared<GameObject>(name, id, txtFilePath);
		auto& transform = obj->GetTransform();
		transform->SetPosition(pos);
		transform->SetRotation(rot);
		transform->SetScale(scale);
		obj->SetEnableState(isEnabled);

		GLuint VAO, VBO, EBO;

		glGenVertexArrays(1, &VAO);
		glGenBuffers(1, &VBO);
		glGenBuffers(1, &EBO);

		glBindVertexArray(VAO);

		glBindBuffer(GL_ARRAY_BUFFER, VBO);

		glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(AssimpVertex), &vertices[0], GL_STATIC_DRAW);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);

		//vertex Positions
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(AssimpVertex), (void*)0);
		//vertex normals
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(AssimpVertex), (void*)offsetof(AssimpVertex, normal));
		//vertex texture coords
		glEnableVertexAttribArray(2);
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(AssimpVertex), (void*)offsetof(AssimpVertex, texCoords));
		//vertex tangent
		glEnableVertexAttribArray(3);
		glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(AssimpVertex), (void*)offsetof(AssimpVertex, tangent));
		//vertex bitangent
		glEnableVertexAttribArray(4);
		glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(AssimpVertex), (void*)offsetof(AssimpVertex, bitangent));
		//ids
		glEnableVertexAttribArray(5);
		glVertexAttribIPointer(5, 4, GL_INT, sizeof(AssimpVertex), (void*)offsetof(AssimpVertex, boneIDs));
		//weights
		glEnableVertexAttribArray(6);
		glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, sizeof(AssimpVertex), (void*)offsetof(AssimpVertex, weights));
		glBindVertexArray(0);

		auto mesh = obj->AddComponent<MeshComponent>(
			MeshType::model,
			VAO, 
			VBO, 
			EBO);

		string vert = (path(Engine::filesPath) / "shaders" / "GameObject.vert").string();
		string frag = (path(Engine::filesPath) / "shaders" / "GameObject.frag").string();

		if (!exists(vert)
			|| !exists(frag))
		{
			Engine::CreateErrorPopup("One of the shader paths for model is invalid!");
		}

		Shader modelShader = Shader::LoadShader(vert, frag);

		auto mat = obj->AddComponent<MaterialComponent>();
		mat->AddShader(vert, frag, modelShader);

		mesh->SetVertices(vertices);
		mesh->SetIndices(indices);

		Texture::LoadTexture(obj, diffTexture, MaterialComponent::TextureType::diffuse, false);
		Texture::LoadTexture(obj, specTexture, MaterialComponent::TextureType::specular, false);
		Texture::LoadTexture(obj, "EMPTY", MaterialComponent::TextureType::height, false);
		Texture::LoadTexture(obj, "EMPTY", MaterialComponent::TextureType::normal, false);

		Shader assignedShader = mat->GetShader();
		assignedShader.Use();
		assignedShader.SetInt("material.diffuse", 0);
		assignedShader.SetInt("material.specular", 1);

		obj->SetTxtFilePath(txtFilePath);

		GameObjectManager::AddGameObject(obj);
		GameObjectManager::AddOpaqueObject(obj);

#if ENGINE_MODE
		GUISceneWindow::UpdateCounts();
#endif
		GameObjectFile::LoadModel(txtFilePath);

		Select::selectedObj = obj;
		Select::isObjectSelected = true;

		ConsoleManager::WriteConsoleMessage(
			Caller::FILE,
			Type::DEBUG,
			"Successfully initialized Model with name " + obj->GetName() + " and ID " + to_string(obj->GetID()) + "\n");

		return obj;
	}

	void Model::Render(
		const shared_ptr<GameObject>& obj,
		const mat4& view,
		const mat4& projection)
	{
		if (obj == nullptr) Engine::CreateErrorPopup("Model gameobject is invalid.");

		if (obj->IsEnabled())
		{
			auto mat = obj->GetComponent<MaterialComponent>();
			if (!mat) return;

			Shader shader = mat->GetShader();

			shader.Use();
			shader.SetVec3("viewPos", Render::camera.GetCameraPosition());
			shader.SetFloat("material.shininess", 2);

			//directional light
			if (GameObjectManager::GetDirectionalLight() != nullptr)
			{
				shared_ptr<GameObject> dirLight = GameObjectManager::GetDirectionalLight();

				int count = dirLight->IsEnabled() ? 1 : 0;
				shader.SetInt("dirLightCount", count);
				
				auto& transform = dirLight->GetTransform();
				auto light = dirLight->GetComponent<LightComponent>();
				if (light)
				{
					vec3 dirRot = transform->GetRotation();
					quat dirQuat = quat(radians(dirRot));
					//assuming the initial direction is along the negative Y-axis
					vec3 initialDir = vec3(0.0f, -1.0f, 0.0f);
					//rotate the initial direction using the quaternion
					vec3 rotatedDir = dirQuat * initialDir;
					rotatedDir = normalize(rotatedDir);

					shader.SetBool("dirLight.enabled", dirLight->IsEnabled());

					shader.SetVec3("dirLight.direction", rotatedDir);

					shader.SetFloat("dirLight.intensity", light->GetIntensity());
					shader.SetVec3("dirLight.ambient", 0.05f, 0.05f, 0.05f);
					shader.SetVec3("dirLight.diffuse", light->GetDiffuse());
					shader.SetVec3("dirLight.specular", 0.5f, 0.5f, 0.5f);
				}
				else
				{
					shader.SetBool("dirLight.enabled", false);
				}
			}
			else 
			{
				shader.SetBool("dirLight.enabled", false);
			}

			//point lights
			const vector<shared_ptr<GameObject>>& pointLights = GameObjectManager::GetPointLights();
			int pointLightCount = static_cast<int>(pointLights.size());
			shader.SetInt("pointLightCount", pointLightCount);
			if (pointLightCount > 0)
			{
				for (int i = 0; i < pointLightCount; i++)
				{
					auto& transform = pointLights[i]->GetTransform();
					auto light = pointLights[i]->GetComponent<LightComponent>();
					if (light)
					{
						string lightPrefix = "pointLights[" + to_string(i) + "].";

						shader.SetBool(lightPrefix + "enabled", pointLights[i]->IsEnabled());

						shader.SetVec3(lightPrefix + "position", transform->GetPosition());
						shader.SetVec3(lightPrefix + "ambient", 0.05f, 0.05f, 0.05f);
						shader.SetVec3(lightPrefix + "diffuse", light->GetDiffuse());
						shader.SetVec3(lightPrefix + "specular", 1.0f, 1.0f, 1.0f);
						shader.SetFloat(lightPrefix + "constant", 1.0f);
						shader.SetFloat(lightPrefix + "linear", 0.09f);
						shader.SetFloat(lightPrefix + "quadratic", 0.032f);
						shader.SetFloat(lightPrefix + "intensity", light->GetIntensity());
						shader.SetFloat(lightPrefix + "distance", light->GetDistance());
					}
					else
					{
						string lightPrefix = "pointLights[" + to_string(i) + "].";

						shader.SetBool(lightPrefix + "enabled", false);
					}
				}
			}

			//spotlights
			const vector<shared_ptr<GameObject>>& spotLights = GameObjectManager::GetSpotLights();
			int spotLightCount = static_cast<int>(spotLights.size());
			shader.SetInt("spotLightCount", spotLightCount);
			if (spotLightCount > 0)
			{
				for (int i = 0; i < spotLightCount; i++)
				{
					auto& transform = spotLights[i]->GetTransform();
					auto light = spotLights[i]->GetComponent<LightComponent>();
					if (light)
					{
						string lightPrefix = "spotLights[" + to_string(i) + "].";
						shader.SetVec3(lightPrefix + "position", transform->GetPosition());

						shader.SetBool(lightPrefix + "enabled", spotLights[i]->IsEnabled());

						vec3 rotationAngles = transform->GetRotation();
						quat rotationQuat = quat(radians(rotationAngles));
						//assuming the initial direction is along the negative Y-axis
						vec3 initialDirection = vec3(0.0f, -1.0f, 0.0f);
						//rotate the initial direction using the quaternion
						vec3 rotatedDirection = rotationQuat * initialDirection;
						//set the rotated direction in the shader
						shader.SetVec3(lightPrefix + "direction", rotatedDirection);

						shader.SetFloat(lightPrefix + "intensity", light->GetIntensity());
						shader.SetFloat(lightPrefix + "distance", light->GetDistance());
						shader.SetVec3(lightPrefix + "ambient", 0.0f, 0.0f, 0.0f);
						shader.SetVec3(lightPrefix + "diffuse", light->GetDiffuse());
						shader.SetVec3(lightPrefix + "specular", 1.0f, 1.0f, 1.0f);
						shader.SetFloat(lightPrefix + "constant", 1.0f);
						shader.SetFloat(lightPrefix + "linear", 0.09f);
						shader.SetFloat(lightPrefix + "quadratic", 0.032f);
						shader.SetFloat(lightPrefix + "cutOff", cos(radians(light->GetInnerAngle())));
						shader.SetFloat(lightPrefix + "outerCutOff", cos(radians(light->GetOuterAngle())));
					}
					else 
					{
						string lightPrefix = "spotLights[" + to_string(i) + "].";
						shader.SetVec3(lightPrefix + "position", transform->GetPosition());

						shader.SetBool(lightPrefix + "enabled", false);
					}
				}
			}

			shader.SetMat4("projection", projection);
			shader.SetMat4("view", view);

			mat4 model = mat4(1.0f);

			model = translate(model, obj->GetTransform()->GetPosition());

			quat newRot = quat(radians(obj->GetTransform()->GetRotation()));
			model *= mat4_cast(newRot);

			model = scale(model, obj->GetTransform()->GetScale());

			//bind diffuse texture
			unsigned int diffuseTextureID = mat->GetTextureID(MaterialComponent::TextureType::diffuse);
			if (diffuseTextureID != 0)
			{
				glActiveTexture(GL_TEXTURE0);
				glBindTexture(GL_TEXTURE_2D, diffuseTextureID);
			}

			//bind specular texture
			unsigned int specularTextureID = mat->GetTextureID(MaterialComponent::TextureType::specular);
			if (specularTextureID != 0)
			{
				glActiveTexture(GL_TEXTURE1);
				glBindTexture(GL_TEXTURE_2D, specularTextureID);
			}

			shader.SetMat4("model", model);

			auto mesh = obj->GetComponent<MeshComponent>();
			GLuint VAO = mesh->GetVAO();
			glBindVertexArray(VAO);
			glDrawElements(
				GL_TRIANGLES,
				static_cast<unsigned int>(mesh->GetIndices().size()),
				GL_UNSIGNED_INT,
				0);

			glActiveTexture(GL_TEXTURE0);
		}
	}
}
