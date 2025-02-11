#pragma once
#include "GameObject.h"
#include "Collectable.h"
#include <functional>

namespace NCL {
	namespace CSC8503 {
		class Player : public GameObject {
		public:
			Player(string name = "player");
			bool hasCollectable() {return collectedItem != nullptr;};
			void setCollectable(Collectable* c) { collectedItem = c; };
			void removeCollectable() { collectedItem = nullptr; };

			Collectable* getCollected() { return collectedItem; };

			int getScore() {return score;};
			void addScore(int s) { score += s; };

			bool isSwimming() { return swimming; };
			void swapMoveType() { swimming = !swimming; };
			int jumps = 2;
		private:
			
			Collectable* collectedItem = nullptr;
			int score = 0;
			bool swimming = false;
			
		};
	}
}


