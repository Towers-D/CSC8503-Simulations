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

			bool isPlayer() const {
				return player;
			}

			void swapPlayer() {
				player = !player;
			}

			bool isCollectable() {
				return collectable;
			}

			void swapCollectable() {
				collectable = !collectable;
				if (collectable == false)
					lambda = NULL;
				else {
					lambda = [this](GameObject* g) {
						if (g->isPlayer() && g->collected == nullptr) {
							this->GetTransform().SetWorldPosition(g->GetTransform().GetWorldPosition() + Vector3(0, 5, 0));
							this->GetPhysicsObject()->SetInverseMass(0);
							g->collected = this;
							lambda = NULL;
						}
					};
				}
			}

			virtual void OnCollisionBegin(GameObject* otherObject) {
				if (lambda != NULL) {
					lambda(otherObject);
				}
			}

			virtual void OnCollisionEnd(GameObject* otherObject) {
				//std::cout << "OnCollisionEnd event occured!\n";
			}

			bool GetBroadphaseAABB(Vector3&outsize) const;

			void UpdateBroadphaseAABB();

			GameObject* collected = nullptr;

			void addScore(int score) { this->score += score; };
			int getScore() {return score; };

		protected:
			Transform			transform;

			CollisionVolume*	boundingVolume;
			PhysicsObject*		physicsObject;
			RenderObject*		renderObject;
			NetworkObject*		networkObject;

			bool	isActive;
			bool	player = false;
			bool	collectable = false;

			int score = 0;

			std::function<void(GameObject* g)> lambda = NULL;
			string	name;

			Vector3 broadphaseAABB;
		};
	}
}

