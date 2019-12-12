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
	angle = ang;
}

RotationConstraint::~RotationConstraint() {

}

void RotationConstraint::UpdateConstraint(float dt) {
	Vector3 posA = objectA->GetConstTransform().GetWorldPosition();
	Vector3 posB = objectB->GetConstTransform().GetWorldPosition();
	
	Vector3 scaleB = objectB->GetTransform().GetLocalScale();

	Vector3 dist = posA.GetDirection(posB);

	float xRot = atan2(dist.z, dist.x) * (180 / Maths::PI);
	float yRot = atan2(dist.y, dist.x) * (180 / Maths::PI);
	float zRot = atan2(dist.y, dist.z) * (180 / Maths::PI);

	if (xRot < 0) { xRot += 360; };
	if (xRot > 360) { xRot -= 360; };

	PhysicsObject* physA = objectA->GetPhysicsObject();
	PhysicsObject* physB = objectB->GetPhysicsObject();
	
	Vector3 BVel = physB->GetLinearVelocity();
	Vector3 relVel = physA->GetLinearVelocity() - BVel;
	
	float totMass = physA->GetInverseMass() + physB->GetInverseMass();

	if (totMass > 0) {
		float velDot = Vector3::Dot(relVel, dist);
		float biasFactor = 0.01f;
		Vector3 aImpulse = Vector3(0, 0, 0);
		Vector3 bImpulse = Vector3(0, 0, 0);
		
		
		if (sqrt(pow(yRot,2) + 32400.0f) > 0.1) {
			float bias = -(biasFactor / dt) * (180 - yRot);
			float lambda = -(velDot + bias) / totMass;
			aImpulse.y = dist.y * lambda;
			bImpulse.y = -dist.y * lambda * 0.1;
		}

		float xDiff = xRot - 180;
		float ang = 180;
		if (abs(xDiff) > angle)
			ang = xRot - (xDiff - angle);
		else
			ang = -xDiff;

		//Apply impulse so gate wills lowly return to normal
		if (xRot != 180)
			bImpulse.z = ((xRot / 180) - 1) * 2;

		objectB->GetTransform().SetLocalOrientation(Quaternion(Matrix4::Rotation(ang, Vector3(0, 1, 0))));
		physA->ApplyLinearImpulse(aImpulse * 0);
		physB->ApplyLinearImpulse(bImpulse * 0.1);
	}
}
