#include "Enemy.h"
#include "Debug.h"

using namespace NCL;
using namespace CSC8503;

NavigationGrid Enemy::lake = NavigationGrid("LakeGrid.txt");

Enemy::Enemy(string name, Player* player) {
	this->name = name;
	playerPointer = player;

	beginFunction = [this](GameObject* g) {
		if (Player * p = dynamic_cast<Player*>(g)) {
			if (p->hasCollectable()) {
				if (p->getCollected()->isBonus()) {
					p->getCollected()->returnToStart();
					p->getCollected()->GetPhysicsObject()->ClearForces();
					p->getCollected()->GetPhysicsObject()->SetAngularVelocity(Vector3(0,0,0));
					p->getCollected()->GetPhysicsObject()->SetLinearVelocity(Vector3(0,0,0));
					p->removeCollectable();
				}
			}
		}
	};
}

Enemy::~Enemy() {
	delete machine;
}

void Enemy::setUpStateMachine() {
	distance = this->GetTransform().GetWorldPosition().DistanceBetween(playerPointer->GetTransform().GetWorldPosition());
	startDist = this->GetTransform().GetWorldPosition().DistanceBetweenNoY(startPos);
	if (playerPointer->hasCollectable())
		bonus = (playerPointer->getCollected()->isBonus());
	else
		bonus = false;

	StateFunc iFunc = [](void* data) {
		*((State*)data) = IDLE;
	};
	StateFunc wFunc = [](void* data) {
		*((State*)data) = WATCHING;
	};
	StateFunc cFunc = [](void* data) {
		*((State*)data) = CHASING;
	};
	StateFunc rFunc = [](void* data) {
		*((State*)data) = RETURN;
	};

	GenericState* idle = new GenericState(iFunc, (void*)& this->currentState);
	GenericState* watching = new GenericState(wFunc, (void*)& this->currentState);
	GenericState* chasing = new GenericState(cFunc, (void*)& this->currentState);
	GenericState* returning = new GenericState(rFunc, (void*)& this->currentState);
	machine->AddState(idle);
	machine->AddState(watching);
	machine->AddState(chasing);
	machine->AddState(returning);

	GenericTransition<bool&, bool>* ItoW = new GenericTransition<bool&, bool>(GenericTransition<bool&, bool>::EqualsTransition, bonus, true, idle, watching);
	GenericTransition<bool&, bool>* WtoR = new GenericTransition<bool&, bool>(GenericTransition<bool&, bool>::NotEqualsTransition, bonus, true, watching, returning);
	GenericTransition<bool&, bool>* CtoR = new GenericTransition<bool&, bool>(GenericTransition<bool&, bool>::NotEqualsTransition, bonus, true, chasing, returning);
	GenericTransition<bool&, bool>* RtoW = new GenericTransition<bool&, bool>(GenericTransition<bool&, bool>::EqualsTransition, bonus, true, returning, watching);
	GenericTransition<float&, float>* WtoC = new GenericTransition<float&, float>(GenericTransition<float&, float>::LessThanTransition, distance, 30.0f, watching, chasing);
	GenericTransition<float&, float>* CtoW = new GenericTransition<float&, float>(GenericTransition<float&, float>::GreaterThanTransition, distance, 35.0f, chasing, watching);
	GenericTransition<float&, float>* RtoI = new GenericTransition<float&, float>(GenericTransition<float&, float>::LessThanTransition, startDist, 1.0f, returning, idle);
	machine->AddTransition(ItoW);
	machine->AddTransition(WtoR);
	machine->AddTransition(CtoR);
	machine->AddTransition(RtoW);
	machine->AddTransition(WtoC);
	machine->AddTransition(CtoW);
	machine->AddTransition(RtoI);
}

void Enemy::UpdateState(float time) {
	distance = this->GetTransform().GetWorldPosition().DistanceBetween(playerPointer->GetTransform().GetWorldPosition());
	startDist = this->GetTransform().GetWorldPosition().DistanceBetweenNoY(startPos);
	
	if (playerPointer->hasCollectable())
		bonus = (playerPointer->getCollected()->isBonus());
	else
		bonus = false;
	machine->Update();

	switch (currentState) {
		case CHASING:
			genPath(playerPointer->GetTransform().GetWorldPosition(), time);
			break;
		case RETURN:
			genPath(startPos, time);
			break;
		case WATCHING:
			lookAt(playerPointer->GetTransform().GetWorldPosition());
	}
}

void Enemy::genPath(Vector3 endPos, float time) {
	int milli = (time - floor(time)) * 100;
	if (milli % 20 < 3) {
		nodeList.clear();
		path.Clear();
		lookAt(endPos);
		if (lake.getClosestNodeType(playerPointer->GetTransform().GetWorldPosition()) == '.' || currentState == RETURN) {
			bool found = lake.FindPath(this->GetTransform().GetWorldPosition(), endPos, this->path);
			Vector3 pos;
			while (this->path.PopWaypoint(pos))
				nodeList.push_back(pos);
			for (int i = 1; i < nodeList.size(); ++i) {
				Vector3 a = nodeList[i - 1];
				Vector3 b = nodeList[i];
				a.y += 5;
				b.y += 5;
				Debug::DrawLine(a, b, Vector4(0, 1, 0, 1));
			}
		}
	}

	if (!nodeList.empty()) {
		if (lake.getClosestNodePos(this->GetTransform().GetWorldPosition()) == nodeList[0]) {
			nodeList.erase(std::remove(nodeList.begin(), nodeList.end(), nodeList[0]), nodeList.end());
		}
		if (!nodeList.empty()) {
			Vector3 dir = this->GetTransform().GetWorldPosition().GetDirection(nodeList[0]).Normalised();
			this->GetPhysicsObject()->AddForce(dir * 100);
		}
	}
}

void Enemy::lookAt(Vector3 pos) {
	Vector3 dirV = this->GetTransform().GetWorldPosition().GetDirection(pos);
	float rads = atan2(dirV.x, dirV.z);
	float deg = rads * (180 / 3.141592654);
	Quaternion q = Matrix4::Rotation(deg, Vector3(0, 1, 0));
	this->GetTransform().SetLocalOrientation(q);
}

string Enemy::getState() {
	switch (currentState) {
		case IDLE:
			return "IDLE";
		case WATCHING:
			return "WATCHING";
		case CHASING:
			return "CHASING";
		case RETURN:
			return "RETURN";
	}
}