#pragma once
#include "GameObject.h"
#include "NavigationPath.h"
#include "NavigationGrid.h"

#include "StateMachine.h"
#include "StateTransition.h"
#include "State.h"
#include "Player.h"

 namespace NCL {
	 namespace CSC8503 {
		class Enemy : public GameObject{
		public:
			static NavigationGrid lake;
			enum State {
				IDLE,
				WATCHING,
				CHASING,
				RETURN
			};

			Enemy(string name);
			~Enemy();
			
			string getState();
			
			void genPath(Vector3 endPos, float time);

			void setUpStateMachine();
			void UpdateState(float time, Player* p);
		private:
			State currentState = IDLE;
			NavigationPath path;
			
			void lookAt(Vector3 pos);
			StateMachine* machine = new StateMachine();

			std::vector<Vector3> nodeList;

			float distance = 0;
			float startDist = 0;
			bool bonus = 0;
		};
	}
}


