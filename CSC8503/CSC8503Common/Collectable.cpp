#include "Collectable.h"
#include "Player.h"

using namespace NCL;
using namespace CSC8503;
int Collectable::pointsToCollect;

Collectable::Collectable(string name, bool bonus, int points) {
	this->name = name;
	this->bonus = bonus;
	this->points = points;
	this->basicObject = false;
}

void Collectable::dropped() {
	this->GetPhysicsObject()->SetInverseMass(startingInMass);
	beginFunction = [this](GameObject* g) {
		if (g->GetName() == "Island") {
			this->swapCollectable();
			Collectable::setPoints(this->getPoints());
			beginFunction = NULL;
		}
	};
}

void Collectable::returnToStart() {
	this->GetTransform().SetWorldPosition(startPos);
}