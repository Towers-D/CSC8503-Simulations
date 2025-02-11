#include "Player.h"

using namespace NCL;
using namespace CSC8503;

Player::Player(string name) {
	this->name = name;
	this->basicObject = false;
	beginFunction = [this](GameObject* c) {
		if (jumps != 2)
			jumps = 2;
		if (c->GetName() == "Lake") {
			if (!this->isSwimming())
				this->swapMoveType();
		}
		else if (Collectable* collectable = dynamic_cast<Collectable*>(c)) {
			if (collectable->isCollectable()) {
				this->setCollectable(collectable);
				collectable->swapCollectable();
				collectable->GetTransform().SetWorldPosition(collectable->GetTransform().GetWorldPosition() + Vector3(0, 3, 0));
				collectable->GetPhysicsObject()->SetInverseMass(20.0f);
			}
		}
	};
	endFunction = [this](GameObject* c) {
		if (c->GetName() == "Lake") {
			if (this->isSwimming())
				this->swapMoveType();
		}
	};
}