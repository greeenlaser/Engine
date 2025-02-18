#include "physics.hpp"

#include "glfw3.h"

namespace Core {
   void InitilizePhysicsWorld() {
      RegisterDefaultAllocator();
      Factory::sInstance = new Factory();
      RegisterTypes();

      physicsWorld.allocator = std::unique_ptr<TempAllocatorImpl>(new TempAllocatorImpl(100*1024*1024));

      physicsWorld.jobSystem = std::unique_ptr<JobSystemThreadPool>(new JobSystemThreadPool(cMaxPhysicsJobs, cMaxPhysicsBarriers, std::thread::hardware_concurrency() - 1));

      physicsWorld.physicsSystem.Init(
         physicsWorld.maxBodies,
         physicsWorld.numBodyMutexes, 
         physicsWorld.maxBodyPairs, 
         physicsWorld.maxContactConstraints,
         physicsWorld.broad_phase_layer_interface,
         physicsWorld.object_vs_broadphase_layer_filter,
         physicsWorld.object_vs_object_layer_filter
      );
   }

   void UpdatePhysicsWorld() {
      static float pt = 0;
      float time = glfwGetTime();
      physicsWorld.deltaTime = time-pt;
      pt = time;

      static float accumulator = 0;
      accumulator += physicsWorld.deltaTime;

      while (accumulator >= physicsWorld.fixedTimeStep) {
         // here you want to set the rotation, position, and all other settings of the bodys so that jolt
         // can recognize it for the next physics update
         accumulator -= physicsWorld.fixedTimeStep;

         physicsWorld.physicsSystem.Update(physicsWorld.fixedTimeStep, 1, physicsWorld.allocator.get(), physicsWorld.jobSystem.get());
         
         // now you want to set the position of the object to the position of the jolt body
         // that was just updated, if you want to see how i do this you can see here:
         // https://github.com/tkpmonke/Ignition/blob/indev/src/ignition/modules/physics/world.cpp
         // and here
         // https://github.com/tkpmonke/Ignition/blob/indev/src/runtime/main.cpp
         
         // what i do is pass two std::function's into the updatephysicsworld function and call them before
         // and after the physics world updates. the std::functions contains a loop to loop over every object
         // and calls there FixedUpdate and LateFixedUpdate functions. my rigidbody class then sets the position
         // and rotation and what not in these functions.
         
         // if you want more info just ask me on discord ig
      }
   }

   void ShutdownPhysicsWorld() {
      UnregisterTypes();
      delete Factory::sInstance;
      Factory::sInstance = nullptr;
   }
};
