#pragma once
#include "GameTechRenderer.h"
#include "../CSC8503Common/PhysicsSystem.h"
#include "../CSC8503Common/GameWorld.h"
#include "../../Plugins/OpenGLRendering/OGLMesh.h"
#include "../../Plugins/OpenGLRendering/OGLShader.h"
#include "../../Plugins/OpenGLRendering/OGLTexture.h"
#include "../../Common/TextureLoader.h"

#include "../CSC8503Common/NavigationGrid.h"
#include "../CSC8503Common/PositionConstraint.h"
#include "../CSC8503Common/RotationConstraint.h"
#include "../CSC8503Common/Player.h"
#include "../CSC8503Common/Collectable.h"
#include "../CSC8503Common/Enemy.h"

#include "../CSC8503Common/GameServer.h"
#include "../CSC8503Common/GameClient.h"

namespace NCL {
	namespace CSC8503 {

		class TestPacketReciever : public PacketReceiver {
		public:
			TestPacketReciever(string name) {
				this->name = name;
			}

			void ReceivePacket(int type, GamePacket* payload, int source) {
				if (type == String_Message) {
					StringPacket* realPacket = (StringPacket*)payload;
					string msg = realPacket->GetStringFromData();
					this->msg = msg;
				}
				else if (type == Player_Connected) {
					NewPlayerPacket* realPacket = (NewPlayerPacket*)payload;
					recieved = realPacket->playerID;
				}
			}

			string getString() { return msg; };
			int getID() { return recieved; };
		protected:
			string name;
			string msg;
			int recieved = -1;
		};

		class TutorialGame		{

			enum GameState {
				Menu,
				Single,
				Client,
				Server,
				MultiS,
				MultiC,
				Results
			};

		public:
			TutorialGame(int width, int height);
			~TutorialGame();

			virtual void UpdateGame(float dt);

		protected:
			void InitialiseAssets();

			void InitCamera();
			void UpdateKeys();

			void InitWorld();
			void updateMenu();
			void updateState();

			int menuPos = 1;

			GameState currState = Menu;

			/*
			These are some of the world/object creation functions I created when testing the functionality
			in the module. Feel free to mess around with them to see different objects being created in different
			test scenarios (constraints, collision types, and so on). 
			*/
			void InitSphereGridWorld(int numRows, int numCols, float rowSpacing, float colSpacing, float radius);
			void InitMixedGridWorld(int numRows, int numCols, float rowSpacing, float colSpacing);
			void InitCubeGridWorld(int numRows, int numCols, float rowSpacing, float colSpacing, const Vector3& cubeDims);
			void BridgeConstraintTest(Vector3 startPos = Vector3(50, 50, 50));
			void SimpleGJKTest();

			bool SelectObject();
			void MoveSelectedObject();
			void DebugObjectMovement();
			void LockedObjectMovement();
			void LockedCameraMovement();
			void PlayerMovement();

			GameObject* AddFloorToWorld(const Vector3& position, const Vector3& dimensions);
			GameObject* AddSphereToWorld(const Vector3& position, float radius, float inverseMass = 10.0f);
			GameObject* AddCubeToWorld(const Vector3& position, Vector3 dimensions, string name = "Cube", float inverseMass = 10.0f, Vector4 colour = Vector4(1, 1, 1, 1));
			GameObject* AddOBBCubeToWorld(const Vector3& position, Vector3 dimensions, Quaternion q, string name = "OBB", float inverseMass = 10.0f, Vector4 colour = Vector4(1, 1, 1, 1));
			GameObject* AddLakeToWorld(const Vector3& position, Vector3 dimensions, float inverseMass = 10.0f);
			GameObject* AddIslandToWorld();
			void InitBoundaries();
			void InitBridge();
			void InitGate();
			//IT'S HAPPENING
			Player* AddGooseToWorld(const Vector3& position);
			GameObject* AddParkKeeperToWorld(const Vector3& position);
			Enemy* AddCharacterToWorld(const Vector3& position);
			Collectable* AddAppleToWorld(const Vector3& position);
			Collectable* AddBonusToWorld(const Vector3& position, Vector3 dimensions, float inverseMass = 10.0f);

			GameTechRenderer*	renderer;
			PhysicsSystem*		physics;
			GameWorld*			world;

			Player* player;
			Player* player2;
			Enemy* chaser;
			PositionConstraint* playColl = nullptr;

			bool useGravity;
			bool inSelectionMode;
			bool playing = false;

			float	forceMagnitude;
			float	timeRemaining = 180;
			float	gameTime = 0;

			int screenWidth;
			int screenHeight;

			GameObject* selectionObject = nullptr;

			OGLMesh*	cubeMesh	= nullptr;
			OGLMesh*	sphereMesh	= nullptr;
			OGLTexture* basicTex	= nullptr;
			OGLTexture* blankTex = nullptr;
			OGLShader*	basicShader = nullptr;

			//Coursework Meshes
			OGLMesh*	gooseMesh	= nullptr;
			OGLMesh*	keeperMesh	= nullptr;
			OGLMesh*	appleMesh	= nullptr;
			OGLMesh*	charA		= nullptr;
			OGLMesh*	charB		= nullptr;

			//Coursework Additional functionality	
			GameObject* lockedObject	= nullptr;
			Vector3 lockedOffset		= Vector3(0, 14, 20);
			void LockCameraToObject(GameObject* o) {
				lockedObject = o;
			}

			//Networking
			NetworkBase* base = nullptr;
			GameServer* server = nullptr;
			GameClient* client = nullptr;
			TestPacketReciever serverReceiver = TestPacketReciever("Server");
			TestPacketReciever clientReceiver = TestPacketReciever("Client");
		};
	}
}
