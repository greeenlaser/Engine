//Copyright(C) 2025 Lost Empire Entertainment
//This program comes with ABSOLUTELY NO WARRANTY.
//This is free software, and you are welcome to redistribute it under certain conditions.
//Read LICENSE.md for more information.

#pragma once

#include <memory>
#include <vector>

//external
#include "glm.hpp"

//engine
#include "gameobject.hpp"

namespace Game
{
	using std::shared_ptr;
	using glm::vec3;
	using std::vector;

	using Graphics::Shape::GameObject;

	class Crane
	{
	public:
		static inline bool foundObjects;
		static inline bool isCraneActivated;
		static inline bool isCraneMovingXZ;
		static inline bool isCraneMovingY;

		static inline shared_ptr<GameObject> pickedUpBarrel;
		static inline shared_ptr<GameObject> crane;
		static inline shared_ptr<GameObject> crane_grab;
		static inline shared_ptr<GameObject> craneController;

		static inline vector<shared_ptr<GameObject>> barrels;

		static void LookForObjects();
		static void StartCraneMove(const vec3& addedPos);
		static void MoveCrane();

		static void StartBarrelGrab();
		static void GrabBarrel();
		static shared_ptr<GameObject> GetClosestBarrel();

		static void EnterCrane();
		static void ExitCrane();
	private:
		static inline vec3 craneStartPos = vec3(4.97f, -1.44f, 14.28f);
		static inline vec3 craneGrabStartPos = vec3(4.97f, -1.44f, 14.28f);

		static inline vec3 originalPos;
		static inline vec3 craneMovePos;
		static inline vec3 craneGrabPos;

		static inline bool moveCraneDown;
		static inline bool moveCraneUp;
	};
}