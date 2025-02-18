#pragma once

#include <Jolt.h>
#include <Jolt/RegisterTypes.h>
#include <Jolt/Core/Factory.h>
#include <Jolt/Core/TempAllocator.h>
#include <Jolt/Core/JobSystemThreadPool.h>
#include <Jolt/Physics/PhysicsSettings.h>
#include <Jolt/Physics/PhysicsSystem.h>
#include <Jolt/Physics/Collision/Shape/BoxShape.h>
#include <Jolt/Physics/Collision/Shape/SphereShape.h>
#include <Jolt/Physics/Collision/Shape/EmptyShape.h>
#include <Jolt/Physics/Collision/ShapeCast.h>
#include <Jolt/Physics/Collision/Shape/MeshShape.h>
#include <Jolt/Physics/Body/BodyCreationSettings.h>
#include <Jolt/Physics/Body/BodyActivationListener.h>

using namespace JPH;

namespace Core {
   namespace Layers {
      static constexpr ObjectLayer NonMoving = 0;
      static constexpr ObjectLayer Moving = 1;
      static constexpr ObjectLayer NumOfLayers = 2;
   }
   namespace BroadPhaseLayers
   {
      static constexpr BroadPhaseLayer NON_MOVING(0);
      static constexpr BroadPhaseLayer MOVING(1);
      static constexpr uint NUM_LAYERS(2);
   };
   
   class ObjectLayerPairFilterImpl : public ObjectLayerPairFilter
   {
   public:
   	virtual bool ShouldCollide(ObjectLayer inObject1, ObjectLayer inObject2) const override
   	{
         return true;
   		switch (inObject1)
   		{
            case Layers::NonMoving:
   			return inObject2 == Layers::Moving; // Non moving only collides with moving
   		case Layers::Moving:
   			return true; // Moving collides with everything
   		default:
   			return false;
   		}
   	}
   };
   
   class BPLayerInterfaceImpl final : public BroadPhaseLayerInterface
   {
   public:
   	BPLayerInterfaceImpl()
   	{
   		// Create a mapping table from object to broad phase layer
   		mObjectToBroadPhase[Layers::NonMoving] = BroadPhaseLayers::NON_MOVING;
   		mObjectToBroadPhase[Layers::Moving] = BroadPhaseLayers::MOVING;
   	}
   
   	virtual uint GetNumBroadPhaseLayers() const override
   	{
   		return BroadPhaseLayers::NUM_LAYERS;
   	}
   
   	virtual BroadPhaseLayer	GetBroadPhaseLayer(ObjectLayer inLayer) const override
   	{
   		JPH_ASSERT(inLayer < Layers::NumOfLayers);
   		return mObjectToBroadPhase[inLayer];
   	}
   
   #if defined(JPH_EXTERNAL_PROFILE) || defined(JPH_PROFILE_ENABLED)
   	virtual const char* GetBroadPhaseLayerName(BroadPhaseLayer inLayer) const override
   	{
   		switch ((BroadPhaseLayer::Type)inLayer)
   		{
   		case (BroadPhaseLayer::Type)BroadPhaseLayers::NON_MOVING:	return "NON_MOVING";
   		case (BroadPhaseLayer::Type)BroadPhaseLayers::MOVING:		return "MOVING";
   		default:													JPH_ASSERT(false); return "INVALID";
   		}
   	}
   #endif // JPH_EXTERNAL_PROFILE || JPH_PROFILE_ENABLED
   
   private:
   	BroadPhaseLayer mObjectToBroadPhase[Layers::NumOfLayers];
   };
   
   class ObjectVsBroadPhaseLayerFilterImpl : public ObjectVsBroadPhaseLayerFilter
   {
   public:
   	virtual bool ShouldCollide(ObjectLayer inLayer1, BroadPhaseLayer inLayer2) const override
   	{
         return true;
   		switch (inLayer1)
   		{
   		case Layers::NonMoving:
   			return inLayer2 == BroadPhaseLayers::MOVING;
   		case Layers::Moving:
   			return true;
   		default:
   			JPH_ASSERT(false);
   			return false;
   		}
   	}
   };


   struct PhysicsWorld {
      float deltaTime;
      float fixedTimeStep = (float)1/120;
      int sampleRate;

      int maxBodies = 10000;
      int maxBodyPairs = 65536;
      int numBodyMutexes = 8;
      int maxContactConstraints = 65536;

      PhysicsSystem physicsSystem;
      std::unique_ptr<TempAllocatorImpl> allocator;
      std::unique_ptr<JobSystemThreadPool> jobSystem;

      BPLayerInterfaceImpl broad_phase_layer_interface;
	   ObjectVsBroadPhaseLayerFilterImpl object_vs_broadphase_layer_filter;
	   ObjectLayerPairFilterImpl object_vs_object_layer_filter;
   };

   static inline PhysicsWorld physicsWorld;
   void InitilizePhysicsWorld();
   void UpdatePhysicsWorld();
   void ShutdownPhysicsWorld();
}
