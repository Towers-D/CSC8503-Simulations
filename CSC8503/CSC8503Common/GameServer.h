#pragma once
#include <thread>
#include <atomic>
#include <vector>
#include "NetworkBase.h"

namespace NCL {
	namespace CSC8503 {
		class GameWorld;
		class GameServer : public NetworkBase {
		public:
			GameServer(int onPort, int maxClients);
			~GameServer();

			bool Initialise();
			void Shutdown();

			void SetGameWorld(GameWorld &g);

			//void ThreadedUpdate();

			bool SendGlobalPacket(int msgID);
			bool SendGlobalPacket(GamePacket& packet);

			virtual void UpdateServer();

			int getClientCount() { return clientCount; };
			void incClientCount() { clientCount++; };
			std::vector<int> playerIDs;
		protected:
			int			port;
			int			clientMax;
			int			clientCount;
			GameWorld*	gameWorld;

			
			//std::atomic<bool> threadAlive;

			

			//std::thread updateThread;

			int incomingDataRate;
			int outgoingDataRate;
		};
	}
}
