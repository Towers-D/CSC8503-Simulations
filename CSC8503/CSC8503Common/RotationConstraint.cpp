#include "RotationConstraint.h"



using namespace NCL;
using namespace NCL::Maths;
using namespace CSC8503;

RotationConstraint::RotationConstraint(GameObject* a, GameObject* b, float ang) {
	objectA = a;
	objectB = b;

	Vector3 posA = objectA->GetConstTransform().GetWorldPosition();
	Vector3 posB = objectB->GetConstTransform().GetWorldPosition();
	Vector3 sizes = objectA->GetTransform().GetLocalScale();
	nearestPoint = Maths::Clamp(posB, posA - sizes, posA + sizes);
	angle = ang;
}

RotationConstraint::~RotationConstraint() {

}

void RotationConstraint::UpdateConstraint(float dt) {
	Vector3 posA = objectA->GetConstTransform().GetWorldPosition();
	Vector3 posB = objectB->GetConstTransform().GetWorldPosition();
	
	
	Vector3 offDir = (posA - posB).Normalised();
	Vector3 dist = nearestPoint.GetDirection(posB);

	

	float xRot = atan2(dist.z, dist.x) * (180 / Maths::PI);
	float yRot = atan2(dist.y, dist.x) * (180 / Maths::PI);
	float zRot = atan2(dist.y, dist.z) * (180 / Maths::PI);
	
	PhysicsObject* physA = objectA->GetPhysicsObject();
	PhysicsObject* physB = objectB->GetPhysicsObject();

	Vector3 relVel = physA->GetLinearVelocity() - physB->GetLinearVelocity();
	float totMass = physA->GetInverseMass() + physB->GetInverseMass();

	if (totMass > 0) {
		float velDot = Vector3::Dot(relVel, offDir);
		float biasFactor = 0.01f;
		Vector3 aImpulse = Vector3(0, 0, 0);
		Vector3 bImpulse = Vector3(0, 0, 0);
		std::cout << "y: " << yRot << std::endl;
		std::cout << "x: " << xRot << std::endl;
		std::cout << "z: " << zRot << std::endl;
		if (sqrt(pow(yRot,2) + 32400.0f) > 0.1) {
			float bias = -(biasFactor / dt) * (180 - yRot);
			float lambda = -(velDot + bias) / totMass;
			aImpulse.y = offDir.y * lambda;
			bImpulse.y = -offDir.y * lambda;
		}
		if (abs(xRot) < 180 + angle) {
			float bias = -(biasFactor / dt) * (0 - xRot);
			float lambda = -(velDot + bias) / totMass;
			aImpulse.x = offDir.x * lambda;
			//bImpulse.x = -offDir.x * lambda;
		}
		if (abs(zRot) < 1) {
			float bias = -(biasFactor / dt) * (0 - zRot);
			float lambda = -(velDot + bias) / totMass;
			aImpulse.z = offDir.z * lambda;
			//bImpulse.z = -offDir.z * lambda;
		}
		physA->ApplyLinearImpulse(aImpulse * 0);
		physB->ApplyLinearImpulse(bImpulse * 0.01);
	}
}
