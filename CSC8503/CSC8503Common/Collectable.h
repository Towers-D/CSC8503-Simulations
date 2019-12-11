#pragma once
#include "GameObject.h"

namespace NCL {
	namespace CSC8503 {
		class Collectable : public GameObject {
		public:
			Collectable(string name = "", bool bonus = false, int points = 100);

			void dropped();

			int getPoints() { return points; };
			bool isBonus() { return bonus; };

			void swapCollectable() { collectable = false; };
			bool isCollectable() { return collectable; };

			void returnToStart();
			void setStartMass(float f) {this->startingInMass = f;};
			float getStartMass() { return startingInMass;};

			static void setPoints(int i) { pointsToCollect = i; };
			static int retrievePoints() { return pointsToCollect; };
		private:
			int points;
			bool collectable = true;
			bool bonus;
			float startingInMass = 0;

			static int pointsToCollect;
		};
	}
}


