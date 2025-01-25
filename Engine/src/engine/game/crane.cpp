//Copyright(C) 2025 Lost Empire Entertainment
//This program comes with ABSOLUTELY NO WARRANTY.
//This is free software, and you are welcome to redistribute it under certain conditions.
//Read LICENSE.md for more information.

#include <vector>
#include <string>
#include <iostream>

//engine
#include "crane.hpp"
#include "gameobject.hpp"
#include "timeManager.hpp"

using std::vector;
using std::string;
using std::cout;
using std::to_string;
using glm::length;

using Graphics::Shape::GameObjectManager;
using Core::TimeManager;

namespace Game
{
	void Crane::LookForCraneAndCraneController()
	{
		if (crane == nullptr)
		{
			const vector<shared_ptr<GameObject>>& objects = GameObjectManager::GetObjects();
			for (const auto& obj : objects)
			{
				string name = obj->GetName();
				if (name == "crane")
				{
					crane = obj;
					crane->GetTransform()->SetPosition(craneStartPos);
					break;
				}
			}
		}

		if (crane_grab == nullptr)
		{
			const vector<shared_ptr<GameObject>>& objects = GameObjectManager::GetObjects();
			for (const auto& obj : objects)
			{
				string name = obj->GetName();
				if (name == "crane_grab")
				{
					crane_grab = obj;
					crane_grab->GetTransform()->SetPosition(craneGrabStartPos);
					break;
				}
			}
		}

		if (craneController == nullptr)
		{
			const vector<shared_ptr<GameObject>>& objects = GameObjectManager::GetObjects();
			for (const auto& obj : objects)
			{
				string name = obj->GetName();
				if (name == "craneController")
				{
					craneController = obj;
					break;
				}
			}
		}
	}

	void Crane::StartCraneMove(const vec3& addedPos)
	{
		originalPos = crane->GetTransform()->GetPosition();
		craneMovePos = crane->GetTransform()->GetPosition() + addedPos;

		isCraneMovingXZ = true;
	}

	void Crane::MoveCrane()
	{
		if (isCraneMovingXZ)
		{
			vec3 craneOriginPos = crane->GetTransform()->GetPosition();
			vec3 craneGrabOriginPos = crane_grab->GetTransform()->GetPosition();
			float distance = length(craneOriginPos - craneMovePos);

			if (distance > 0.01f)
			{
				//set direction where crane and crane grab will be moving towards
				vec3 direction = normalize(craneMovePos - craneOriginPos);

				//assign speed multiplier and speed
				float speedMult = 2.0f;
				float speed = TimeManager::deltaTime * speedMult;

				//move crane towards crane target
				vec3 newCranePosition = craneOriginPos + direction * speed;
				crane->GetTransform()->SetPosition(newCranePosition);

				//move crane grab towards crane grab target
				vec3 newCraneGrabPosition = craneGrabOriginPos + direction * speed;
				crane_grab->GetTransform()->SetPosition(newCraneGrabPosition);
			}
			else
			{
				//finally snap crane to target position if distance is very small
				crane->GetTransform()->SetPosition(craneMovePos);

				//and also snap crane grab to target position but with correct y value
				float craneGrabY = crane_grab->GetTransform()->GetPosition().y;
				crane_grab->GetTransform()->SetPosition(vec3(craneMovePos.x, craneGrabY, craneMovePos.z));

				isCraneMovingXZ = false;
			}
		}
	}

	void Crane::StartBarrelGrab()
	{
		originalPos = crane_grab->GetTransform()->GetPosition();
		craneGrabPos = crane_grab->GetTransform()->GetPosition() - vec3(0.0f, 2.0f, 0.0f);

		moveCraneDown = true;
		moveCraneUp = false;
		isCraneMovingY = true;
	}
	void Crane::GrabBarrel()
	{
		if (isCraneMovingY)
		{
			static float craneGrabY = crane_grab->GetTransform()->GetPosition().y;
			if (moveCraneDown)
			{
				vec3 craneGrabOriginPos = crane_grab->GetTransform()->GetPosition();
				float downDistance = length(craneGrabOriginPos - craneGrabPos);

				if (downDistance > 0.01f)
				{
					//set direction where crane grab will move downwards
					vec3 direction = normalize(craneGrabPos - craneGrabOriginPos);

					//assign speed multiplier and speed
					float speedMult = 2.0f;
					float speed = TimeManager::deltaTime * speedMult;

					//move crane grab towards crane grab target (downward movement)
					vec3 newCraneGrabPosition = craneGrabOriginPos + direction * speed;
					crane_grab->GetTransform()->SetPosition(newCraneGrabPosition);
				}
				else
				{
					//snap crane grab to target position with correct Y value
					crane_grab->GetTransform()->SetPosition(vec3(craneGrabPos.x, craneGrabPos.y, craneGrabPos.z));

					//stop moving down and start moving up
					moveCraneDown = false;
					moveCraneUp = true;
				}
			}
			else if (moveCraneUp)
			{
				vec3 craneGrabOriginPos = crane_grab->GetTransform()->GetPosition();
				vec3 craneGrabUpPos = vec3(craneGrabOriginPos.x, craneGrabY, craneGrabOriginPos.z);
				float upDistance = length(craneGrabOriginPos - craneGrabUpPos);

				if (upDistance > 0.01f)
				{
					//set direction where crane grab will move upwards
					vec3 direction = normalize(craneGrabUpPos - craneGrabOriginPos);

					//assign speed multiplier and speed
					float speedMult = 2.0f;
					float speed = TimeManager::deltaTime * speedMult;

					//move crane grab upwards
					vec3 newCraneGrabPosition = craneGrabOriginPos + direction * speed;
					crane_grab->GetTransform()->SetPosition(newCraneGrabPosition);
				}
				else
				{
					//snap crane grab back to the start position
					crane_grab->GetTransform()->SetPosition(craneGrabUpPos);

					//stop movement after reaching the top
					moveCraneUp = false;
					isCraneMovingY = false;
				}
			}

		}
	}

	void Crane::EnterCrane()
	{
		isCraneActivated = true;
	}
	void Crane::ExitCrane()
	{
		isCraneActivated = false;
	}
}