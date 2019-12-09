#include "Player.h"

using namespace NCL;
using namespace CSC8503;

Player::Player(string name) {
	this->name = name;
	this->basicObject = false;
	lambda = [this](GameObject* c) {
		if (Collectable* collectable = dynamic_cast<Collectable*>(c)) {
			if (collectable->isCollectable()) {
				this->setCollectable(collectable);
				collectable->GetTransform().SetWorldPosition(collectable->GetTransform().GetWorldPosition() + Vector3(0, 3, 0));
			}
		}
	};
}