#pragma once
#include "Transform.h"
#include "CollisionVolume.h"
#include <functional>

#include "PhysicsObject.h"
#include "RenderObject.h"
#include "NetworkObject.h"

#include <vector>

using std::vector;

namespace NCL {
	namespace CSC8503 {
		class NetworkObject;

		class GameObject	{
		public:
			GameObject(string name = "");
			~GameObject();

			void SetBoundingVolume(CollisionVolume* vol) {
				boundingVolume = vol;
			}

			const CollisionVolume* GetBoundingVolume() const {
				return boundingVolume;
			}

			bool IsActive() const {
				return isActive;
			}

			const Transform& GetConstTransform() const {
				return transform;
			}

			Transform& GetTransform() {
				return transform;
			}

			RenderObject* GetRenderObject() const {
				return renderObject;
			}

			PhysicsObject* GetPhysicsObject() const {
				return physicsObject;
			}

			NetworkObject* GetNetworkObject() const {
				return networkObject;
			}

			void SetRenderObject(RenderObject* newObject) {
				renderObject = newObject;
			}

			void SetPhysicsObject(PhysicsObject* newObject) {
				physicsObject = newObject;
			}

			const string& GetName() const {
				return name;
			}
			virtual void OnCollisionBegin(GameObject* otherObject) {
				if (beginFunction != NULL) {
					beginFunction(otherObject);
				}
			}

			virtual void OnCollisionEnd(GameObject* otherObject) {
				if (endFunction != NULL) {
					endFunction(otherObject);
				}
			}

			bool GetBroadphaseAABB(Vector3&outsize) const;

			void UpdateBroadphaseAABB();


			bool isBasic() { return basicObject; };
			void setStartingPosition(Vector3 v) { startPos = v; };

		protected:
			Transform			transform;

			CollisionVolume*	boundingVolume;
			PhysicsObject*		physicsObject;
			RenderObject*		renderObject;
			NetworkObject*		networkObject;

			bool	isActive;
			bool	basicObject = true;
			Vector3 startPos;
			
			std::function<void(GameObject* g)> beginFunction = NULL;
			std::function<void(GameObject* g)> endFunction = NULL;

			string	name;

			Vector3 broadphaseAABB;
		};
	}
}

