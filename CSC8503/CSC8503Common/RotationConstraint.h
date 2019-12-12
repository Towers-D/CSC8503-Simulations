#pragma once
#include "Constraint.h"
#include "../../Common/Vector3.h"
#include "GameObject.h"
#include "Debug.h"
#include "../../Common/Maths.h"

namespace NCL {
	namespace CSC8503 {
		class GameObject;

		class RotationConstraint : public Constraint{
		public:
			RotationConstraint(GameObject* a, GameObject* b, float angle);
			~RotationConstraint();

			void UpdateConstraint(float dt) override;

		protected:
			GameObject* objectA;
			GameObject* objectB;
			float angle;
		};
	}
}
