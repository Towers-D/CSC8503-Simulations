#include "TutorialGame.h"


using namespace NCL;
using namespace CSC8503;

TutorialGame::TutorialGame(int width, int height)	{
	screenHeight = height;
	screenWidth = width;

	world		= new GameWorld();
	renderer	= new GameTechRenderer(*world);
	physics		= new PhysicsSystem(*world);

	forceMagnitude	= 10.0f;
	useGravity		= false;
	inSelectionMode = false;

	Debug::SetRenderer(renderer);

	InitialiseAssets();
}

/*

Each of the little demo scenarios used in the game uses the same 2 meshes, 
and the same texture and shader. There's no need to ever load in anything else
for this module, even in the coursework, but you can add it if you like!

*/
void TutorialGame::InitialiseAssets() {
	auto loadFunc = [](const string& name, OGLMesh** into) {
		*into = new OGLMesh(name);
		(*into)->SetPrimitiveType(GeometryPrimitive::Triangles);
		(*into)->UploadToGPU();
	};

	loadFunc("cube.msh"		 , &cubeMesh);
	loadFunc("sphere.msh"	 , &sphereMesh);
	loadFunc("goose.msh"	 , &gooseMesh);
	loadFunc("CharacterA.msh", &keeperMesh);
	loadFunc("CharacterM.msh", &charA);
	loadFunc("CharacterF.msh", &charB);
	loadFunc("Apple.msh"	 , &appleMesh);

	basicTex	= (OGLTexture*)TextureLoader::LoadAPITexture("checkerboard.png");
	blankTex = (OGLTexture*)TextureLoader::LoadAPITexture("Blank.png");
	basicShader = new OGLShader("GameTechVert.glsl", "GameTechFrag.glsl");

	InitCamera();
	InitWorld();
}

TutorialGame::~TutorialGame()	{
	delete cubeMesh;
	delete sphereMesh;
	delete gooseMesh;
	delete basicTex;
	delete basicShader;

	delete physics;
	delete renderer;
	delete world;

	delete player;
	delete playColl;
}

void TutorialGame::UpdateGame(float dt) {
	gameTime += dt;
	timeRemaining -= dt;
	float minutes = (timeRemaining / 60);
	int seconds = (minutes - (int) minutes) * 60;
	Debug::Print("Time Remaining: " + std::to_string((int) minutes) + ":" + ((seconds < 10) ? "0" : "" ) + std::to_string(seconds), Vector2(300, 650), Vector4(1,0.5,0,1));
	if (!inSelectionMode) {
		world->GetMainCamera()->UpdateCamera(dt);
	}
	if (lockedObject != nullptr) {
		LockedCameraMovement();
	}

	UpdateKeys();

	if (useGravity) {
		Debug::Print("(G)ravity on", Vector2(10, 40));
	}
	else {
		Debug::Print("(G)ravity off", Vector2(10, 40));
	}

	
	Debug::Print("Player Score: " + std::to_string(player->getScore()), Vector2(10, 60));
	SelectObject();
	MoveSelectedObject();

	chaser->UpdateState(gameTime);
	world->UpdateWorld(dt);
	renderer->Update(dt);
	physics->Update(dt);

	Debug::FlushRenderables();
	renderer->Render();
}

void TutorialGame::UpdateKeys() {
	if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::F1)) {
		InitWorld(); //We can reset the simulation at any time with F1
		selectionObject = nullptr;
	}

	if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::F2)) {
		InitCamera(); //F2 will reset the camera to a specific default place
	}
	if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::F3)) {
		playing = !playing;
		world->GetMainCamera()->swapCam();
	}

	if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::G)) {
		useGravity = !useGravity; //Toggle gravity!
		physics->UseGravity(useGravity);
	}
	//Running certain physics updates in a consistent order might cause some
	//bias in the calculations - the same objects might keep 'winning' the constraint
	//allowing the other one to stretch too much etc. Shuffling the order so that it
	//is random every frame can help reduce such bias.
	if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::F9)) {
		world->ShuffleConstraints(true);
	}
	if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::F10)) {
		world->ShuffleConstraints(false);
	}

	if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::F7)) {
		world->ShuffleObjects(true);
	}
	if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::F8)) {
		world->ShuffleObjects(false);
	}

	if (playing)
		PlayerMovement();
	else if (lockedObject) {
		LockedObjectMovement();
	}
	else {
		DebugObjectMovement();
	}
}

void TutorialGame::LockedObjectMovement() {
	Matrix4 view		= world->GetMainCamera()->BuildViewMatrix();
	Matrix4 camWorld	= view.Inverse();

	Vector3 rightAxis = Vector3(camWorld.GetColumn(0)); //view is inverse of model!

	//forward is more tricky -  camera forward is 'into' the screen...
	//so we can take a guess, and use the cross of straight up, and
	//the right axis, to hopefully get a vector that's good enough!

	Vector3 fwdAxis = Vector3::Cross(Vector3(0, 1, 0), rightAxis);

	if (Window::GetKeyboard()->KeyDown(KeyboardKeys::LEFT)) {
		lockedObject->GetPhysicsObject()->AddForce(-rightAxis * 10);
	}

	if (Window::GetKeyboard()->KeyDown(KeyboardKeys::RIGHT)) {
		lockedObject->GetPhysicsObject()->AddForce(rightAxis * 10);
	}

	if (Window::GetKeyboard()->KeyDown(KeyboardKeys::UP)) {
		lockedObject->GetPhysicsObject()->AddForce(fwdAxis * 10);
	}

	if (Window::GetKeyboard()->KeyDown(KeyboardKeys::DOWN)) {
		lockedObject->GetPhysicsObject()->AddForce(-fwdAxis * 10);
	}
}

void TutorialGame::PlayerMovement() {
	player->addScore(Collectable::retrievePoints());
	Collectable::setPoints(0);
	Vector3 pp = player->GetTransform().GetWorldPosition();
	Vector3 cp = world->GetMainCamera()->GetPosition();

	//Keep Camera Certain Distance from Goose
	float distance = pp.DistanceBetween(cp);
	Vector3 dir = cp.GetDirection(pp);
	dir *= Vector3(1, 0, 1);
	if (distance > 30.5)
		world->GetMainCamera()->SetPosition(world->GetMainCamera()->GetPosition() + dir * (distance - 30.5));
	else if (distance < 29.5)
		world->GetMainCamera()->SetPosition(world->GetMainCamera()->GetPosition() - dir * (29.5 - distance));
	
	//Camera always faces Goose
	Matrix4 view = Matrix4::BuildViewMatrix(cp, pp, Vector3(0, 1, 0));
	Matrix4 modelMat = view.Inverse();
	Quaternion q(modelMat);
	Vector3 angles = q.ToEuler();
	world->GetMainCamera()->SetPitch(angles.x);
	world->GetMainCamera()->SetYaw(angles.y);

	//Goose Movement
	Vector3 rightAxis = Vector3(modelMat.GetColumn(0)); //view is inverse of model!
	Vector3 fwdAxis = Vector3::Cross(Vector3(0, 1, 0), rightAxis);
	float forceMul = player->isSwimming() ? 400: 50;
	int milli = (gameTime - floor(gameTime)) * 100;
	if (!player->isSwimming() || milli < 5) {
		if (Window::GetKeyboard()->KeyDown(KeyboardKeys::A))
			player->GetPhysicsObject()->AddForce(-rightAxis * forceMul);
		if (Window::GetKeyboard()->KeyDown(KeyboardKeys::D))
			player->GetPhysicsObject()->AddForce(rightAxis * forceMul);
		if (Window::GetKeyboard()->KeyDown(KeyboardKeys::W))
			player->GetPhysicsObject()->AddForce(fwdAxis * forceMul);
		if (Window::GetKeyboard()->KeyDown(KeyboardKeys::S))
			player->GetPhysicsObject()->AddForce(-fwdAxis * forceMul);
	}
	if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::SPACE))
		player->GetPhysicsObject()->AddForce(Vector3(0, 750, 0));

	if (player->hasCollectable() && playColl == nullptr) {
		playColl = new PositionConstraint(player, player->getCollected(), 3);
		world->AddConstraint(playColl);
	}

	if (playColl != nullptr && (!player->hasCollectable() || Window::GetKeyboard()->KeyPressed(KeyboardKeys::F))) {
		world->RemoveConstraint(playColl);
		if (player->hasCollectable()) {
			player->getCollected()->dropped();
			player->removeCollectable();
		}
		playColl = nullptr;
	}

	//Goose always faces away from camera
	player->GetTransform().SetLocalOrientation(Matrix4::Rotation(world->GetMainCamera()->GetYaw() + 180, Vector3(0,1,0)));
}

void  TutorialGame::LockedCameraMovement() {
	if (lockedObject != nullptr) {
		Vector3 objPos = lockedObject->GetTransform().GetWorldPosition();
		Vector3 camPos = objPos + lockedOffset;

		Matrix4 temp = Matrix4::BuildViewMatrix(camPos, objPos, Vector3(0, 1, 0));

		Matrix4 modelMat = temp.Inverse();

		Quaternion q(modelMat);
		Vector3 angles = q.ToEuler(); //nearly there now!

		world->GetMainCamera()->SetPosition(camPos);
		world->GetMainCamera()->SetPitch(angles.x -= Window::GetMouse()->GetRelativePosition().y);
		world->GetMainCamera()->SetYaw(angles.y -= Window::GetMouse()->GetRelativePosition().x);
	}
}


void TutorialGame::DebugObjectMovement() {
//If we've selected an object, we can manipulate it with some key presses
	if (inSelectionMode && selectionObject) {
		if (Window::GetKeyboard()->KeyDown(KeyboardKeys::RIGHT)) {
			selectionObject->GetTransform().SetWorldPosition(selectionObject->GetTransform().GetWorldPosition() - Vector3(0, 0, 1));
		}

		if (Window::GetKeyboard()->KeyDown(KeyboardKeys::LEFT)) {
			selectionObject->GetTransform().SetWorldPosition(selectionObject->GetTransform().GetWorldPosition() + Vector3(0, 0, 1));
		}

		if (Window::GetKeyboard()->KeyDown(KeyboardKeys::UP)) {
			selectionObject->GetTransform().SetWorldPosition(selectionObject->GetTransform().GetWorldPosition() - Vector3(1, 0, 0));
		}

		if (Window::GetKeyboard()->KeyDown(KeyboardKeys::DOWN)) {
			selectionObject->GetTransform().SetWorldPosition(selectionObject->GetTransform().GetWorldPosition() + Vector3(1, 0, 0));
		}
		if (Window::GetKeyboard()->KeyDown(KeyboardKeys::PLUS)) {
			selectionObject->GetTransform().SetWorldPosition(selectionObject->GetTransform().GetWorldPosition() + Vector3(0, 1, 0));
		}
		if (Window::GetKeyboard()->KeyDown(KeyboardKeys::MINUS)) {
			selectionObject->GetTransform().SetWorldPosition(selectionObject->GetTransform().GetWorldPosition() - Vector3(0, 1, 0));
		}
		//Object Name
		Debug::Print("Object: " + selectionObject->GetName(), Vector2(20, screenHeight - 50), Vector4(0, 1, 0, 1));

		//Object Position
		Vector3 position = selectionObject->GetTransform().GetWorldPosition();
		string posString = "Position: (" + std::to_string(round(position.x * 100)/100).substr(0, 6) + ", " + std::to_string(round(position.y*100)/100).substr(0, 6) + ", " + std::to_string(round(position.z*100)/100).substr(0, 6) + ")";
		Debug::Print(posString, Vector2(20, screenHeight - 75), Vector4(0, 1, 0, 1));

		//Object Orientation
		Quaternion q = selectionObject->GetTransform().GetLocalOrientation();
		Vector3 Euler = q.ToEuler();
		Debug::Print("Orientation: (" + std::to_string(Euler.x).substr(0, 6) + ", " + std::to_string(Euler.y).substr(0, 6) + ", " + std::to_string(Euler.z).substr(0, 6) + ")", Vector2(20, screenHeight - 100), Vector4(0, 1, 0, 1));


		//Object State
		if (Enemy * E = dynamic_cast<Enemy*>(selectionObject))
			Debug::Print("State: " + E->getState(), Vector2(20, screenHeight - 125), Vector4(0, 1, 0, 1));
	}
}

/*

Every frame, this code will let you perform a raycast, to see if there's an objectVector4(1, 1, 1, 1)
underneath the cursor, and if so 'select it' into a pointer, so that it can be
manipulated later. Pressing Q will let you toggle between this behaviour and instead
letting you move the camera around.

*/
bool TutorialGame::SelectObject() {
	if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::Q)) {
		inSelectionMode = !inSelectionMode;
		if (inSelectionMode) {
			Window::GetWindow()->ShowOSPointer(true);
			Window::GetWindow()->LockMouseToWindow(false);
		}
		else {
			Window::GetWindow()->ShowOSPointer(false);
			Window::GetWindow()->LockMouseToWindow(true);
		}
	}
	if (inSelectionMode) {
		renderer->DrawString("Press Q to change to camera mode!", Vector2(10, 0));

		if (Window::GetMouse()->ButtonDown(NCL::MouseButtons::LEFT)) {
			if (selectionObject) {	//set colour to deselected;
				selectionObject->GetRenderObject()->SetColour(Vector4(1, 1, 1, 1));
				selectionObject = nullptr;
			}

			Ray ray = CollisionDetection::BuildRayFromMouse(*world->GetMainCamera());

			RayCollision closestCollision;
			if (world->Raycast(ray, closestCollision, true)) {
				selectionObject = (GameObject*)closestCollision.node;
				selectionObject->GetRenderObject()->SetColour(Vector4(0, 1, 0, 1));
				return true;
			}
			else {
				return false;
			}
		}
		if (Window::GetKeyboard()->KeyPressed(NCL::KeyboardKeys::L)) {
			if (selectionObject) {
				if (lockedObject == selectionObject) {
					lockedObject = nullptr;
				}
				else {
					lockedObject = selectionObject;
				}
			}
		}
	}
	else {
		renderer->DrawString("Press Q to change to select mode!", Vector2(10, 0));
	}
	return false;
}

/*
If an object has been clicked, it can be pushed with the right mouse button, by an amount
determined by the scroll wheel. In the first tutorial this won't do anything, as we haven't
added linear motion into our physics system. After the second tutorial, objects will move in a straight
line - after the third, they'll be able to twist under torque aswell.
*/

void TutorialGame::MoveSelectedObject() {
	renderer->DrawString("Click Force:" + std::to_string(forceMagnitude), Vector2(10, 20));
	forceMagnitude += Window::GetMouse()->GetWheelMovement() * 100.0f;

	if (!selectionObject)
		return;

	if (Window::GetMouse()->ButtonPressed(NCL::MouseButtons::RIGHT)) {
		Ray ray = CollisionDetection::BuildRayFromMouse(*world->GetMainCamera());
		RayCollision closestCollision;
		if (world->Raycast(ray, closestCollision, true))
			if (closestCollision.node == selectionObject)
				selectionObject->GetPhysicsObject()->AddForceAtPosition(ray.GetDirection() * forceMagnitude, closestCollision.collidedAt);
	}
}

void TutorialGame::InitCamera() {
	world->GetMainCamera()->SetNearPlane(0.5f);
	world->GetMainCamera()->SetFarPlane(2000.0f);
	world->GetMainCamera()->SetPitch(-15.0f);
	world->GetMainCamera()->SetYaw(315.0f);
	world->GetMainCamera()->SetPosition(Vector3(-60, 40, 60));
	lockedObject = nullptr;
}

void TutorialGame::InitWorld() {
	world->ClearAndErase();
	physics->Clear();

	player = AddGooseToWorld(Vector3(-300, 2, 0));
	//AddParkKeeperToWorld(Vector3(40, 5, 0));
	chaser = AddCharacterToWorld(Vector3(45, 5, 0));
	chaser->setUpStateMachine();
	
	Collectable* apple = AddAppleToWorld(Vector3(35, 2, 0));
	Collectable* bonusCube = AddBonusToWorld(Vector3(35, 2, 25), Vector3(0.5, 0.5, 0.5));
	GameObject* lake = AddLakeToWorld(Vector3(-300, -2, 0), Vector3(200, 0.5, 50), 0);
	GameObject* Island = AddIslandToWorld();

	
	InitGate();
	InitBridge();
	InitBoundaries();
	
}

void TutorialGame::InitBoundaries() {
	//Floor Sections
	AddFloorToWorld(Vector3(0, -2, 0), Vector3(100, 2, 100));
	AddFloorToWorld(Vector3(-300, -2, 75), Vector3(200, 2, 25));
	AddFloorToWorld(Vector3(-300, -2, -75), Vector3(200, 2, 25));
	AddFloorToWorld(Vector3(-550, -2, 0), Vector3(50, 2, 100));

	//Long Borders
	AddCubeToWorld(Vector3(-250, 2, -100), Vector3(351, 2, 1), "Border", 0, Vector4(0.42, 0.27, 0.14, 1));
	AddCubeToWorld(Vector3(-250, 2, 100), Vector3(351, 2, 1), "Border", 0, Vector4(0.42, 0.27, 0.14, 1));

	//Short Borders
	AddCubeToWorld(Vector3(100, 2, 0), Vector3(1, 2, 99), "Border", 0, Vector4(0.42, 0.27, 0.14, 1));
	AddCubeToWorld(Vector3(-600, 2, 0), Vector3(1, 2, 99), "Border", 0, Vector4(0.42, 0.27, 0.14, 1));
}

//Generate a Sectioned off area on the side of the park, which has a gate that is constrained
void TutorialGame::InitGate() {
	//Fences Parralell to the Z-Axis
	AddCubeToWorld(Vector3(-350, 1.5, 87.5), Vector3(1, 3, 11.5), "Fence", 0, Vector4(0.42, 0.27, 0.14, 1));
	AddCubeToWorld(Vector3(-290, 1.5, 87.5), Vector3(1, 3, 11.5), "Fence", 0, Vector4(0.42, 0.27, 0.14, 1));

	//Fences Parralell to X-Axis
	AddCubeToWorld(Vector3(-337.25, 1.5, 75), Vector3(13.75, 3, 1), "Fence", 0, Vector4(0.42, 0.27, 0.14, 1));
	AddCubeToWorld(Vector3(-302.75, 1.5, 75), Vector3(13.75, 3, 1), "Fence", 0, Vector4(0.42, 0.27, 0.14, 1));

	//Gate Posts
	GameObject* southPost = AddCubeToWorld(Vector3(-316.5, 2, 75), Vector3(1.1, 3.25, 1.1), "South Post", 0, Vector4(0.42, 0.27, 0.14, 1));
	AddCubeToWorld(Vector3(-323.5, 2, 75), Vector3(1.1, 3.25, 1.1), "North Post", 0, Vector4(0.42, 0.27, 0.14, 1));
	//Gate
	Quaternion q = Matrix4::Rotation(0, Vector3(0,0,0));
	GameObject* gate = AddOBBCubeToWorld(Vector3(-320, 2, 75), Vector3(3.5, 1.5, 0.5), q, "Gate", 0.2f, Vector4(0.42, 0.27, 0.14, 1));

	PositionConstraint* gatePos = new PositionConstraint(southPost, gate, gate->GetTransform().GetWorldPosition().DistanceBetween(southPost->GetTransform().GetWorldPosition()));
	RotationConstraint* hinge = new RotationConstraint(southPost, gate, 105);
	world->AddConstraint(gatePos);
	world->AddConstraint(hinge);
}

void TutorialGame::InitBridge() {
	Quaternion q1 = Matrix4::Rotation(30, Vector3(1, 0, 0));
	Quaternion q2 = Matrix4::Rotation(-30, Vector3(1, 0, 0));

	//Bridge Ramps
	AddOBBCubeToWorld(Vector3(-135, 0, 40), Vector3(5, 5, 15), q1, "Ramp", 0, Vector4(0.42, 0.27, 0.14, 1));
	AddOBBCubeToWorld(Vector3(-135, 0, -40), Vector3(5, 5, 15), q2, "Ramp", 0, Vector4(0.42, 0.27, 0.14, 1));
	
	//Bridge Flateners
	AddCubeToWorld(Vector3(-135, 6.85, -24.5), Vector3(5, 5, 5), "Bridge Level Off", 0, Vector4(0.93, 0.91, 0.67, 1));
	AddCubeToWorld(Vector3(-135, 6.85, 24.5), Vector3(5, 5, 5), "Bridge Level Off", 0, Vector4(0.93, 0.91, 0.67, 1));

	//Bridge Coverings
	AddCubeToWorld(Vector3(-129, 3, -24.5), Vector3(1, 10, 5), "Bridge Pillar", 0, Vector4(0.42, 0.27, 0.14, 1));
	AddCubeToWorld(Vector3(-141, 3, -24.5), Vector3(1, 10, 5), "Bridge Pillar", 0, Vector4(0.42, 0.27, 0.14, 1));
	AddCubeToWorld(Vector3(-129, 3, 24.5), Vector3(1, 10, 5), "Bridge Pillar", 0, Vector4(0.42, 0.27, 0.14, 1));
	AddCubeToWorld(Vector3(-141, 3, 24.5), Vector3(1, 10, 5), "Bridge Pillar", 0, Vector4(0.42, 0.27, 0.14, 1));

	//Bridge Railings
	AddCubeToWorld(Vector3(-129, 10, 0), Vector3(1, 3, 19.5), "Railing", 0, Vector4(0.42, 0.27, 0.14, 1));
	AddCubeToWorld(Vector3(-141, 10, 0), Vector3(1, 3, 19.5), "Railing", 0, Vector4(0.42, 0.27, 0.14, 1));
	
	BridgeConstraintTest(Vector3(-135, 11, 18.5));
}

//From here on it's functions to add in objects to the world!
/*

A single function to add a large immoveable cube to the bottom of our world

*/
GameObject* TutorialGame::AddFloorToWorld(const Vector3& position, const Vector3& dimensions) {
	GameObject* floor = new GameObject("floor");
	
	AABBVolume* volume = new AABBVolume(dimensions);
	floor->SetBoundingVolume((CollisionVolume*)volume);
	floor->GetTransform().SetWorldScale(dimensions);
	floor->GetTransform().SetWorldPosition(position);

	floor->SetRenderObject(new RenderObject(&floor->GetTransform(), cubeMesh, basicTex, basicShader));
	floor->GetRenderObject()->SetColour(Vector4(0.55, 0.71, 0, 1));
	floor->SetPhysicsObject(new PhysicsObject(&floor->GetTransform(), floor->GetBoundingVolume()));

	floor->GetPhysicsObject()->SetInverseMass(0);
	floor->GetPhysicsObject()->InitCubeInertia();

	world->AddGameObject(floor);

	return floor;
}

/*

Builds a game object that uses a sphere mesh for its graphics, and a bounding sphere for its
rigid body representation. This and the cube function will let you build a lot of 'simple' 
physics worlds. You'll probably need another function for the creation of OBB cubes too.

*/
GameObject* TutorialGame::AddSphereToWorld(const Vector3& position, float radius, float inverseMass) {
	GameObject* sphere = new GameObject("sphere");

	Vector3 sphereSize = Vector3(radius, radius, radius);
	SphereVolume* volume = new SphereVolume(radius);
	sphere->SetBoundingVolume((CollisionVolume*)volume);
	sphere->GetTransform().SetWorldScale(sphereSize);
	sphere->GetTransform().SetWorldPosition(position);

	sphere->SetRenderObject(new RenderObject(&sphere->GetTransform(), sphereMesh, basicTex, basicShader));
	sphere->SetPhysicsObject(new PhysicsObject(&sphere->GetTransform(), sphere->GetBoundingVolume()));

	sphere->GetPhysicsObject()->SetInverseMass(inverseMass);
	sphere->GetPhysicsObject()->InitSphereInertia();

	world->AddGameObject(sphere);

	return sphere;
}

GameObject* TutorialGame::AddCubeToWorld(const Vector3& position, Vector3 dimensions, string name, float inverseMass, Vector4 colour) {
	GameObject* cube = new GameObject(name);

	AABBVolume* volume = new AABBVolume(dimensions);

	cube->SetBoundingVolume((CollisionVolume*)volume);

	cube->GetTransform().SetWorldPosition(position);
	cube->GetTransform().SetWorldScale(dimensions);

	cube->SetRenderObject(new RenderObject(&cube->GetTransform(), cubeMesh, basicTex, basicShader));
	cube->SetPhysicsObject(new PhysicsObject(&cube->GetTransform(), cube->GetBoundingVolume()));

	cube->GetPhysicsObject()->SetInverseMass(inverseMass);
	cube->GetPhysicsObject()->InitCubeInertia();

	cube->GetRenderObject()->SetColour(colour);

	world->AddGameObject(cube);

	return cube;
}

GameObject* TutorialGame::AddOBBCubeToWorld(const Vector3& position, Vector3 dimensions, Quaternion q,  string name, float inverseMass, Vector4 colour) {
	GameObject* cube = new GameObject(name);

	OBBVolume* volume = new OBBVolume(dimensions);

	cube->SetBoundingVolume((CollisionVolume*)volume);

	cube->GetTransform().SetWorldPosition(position);
	cube->GetTransform().SetWorldScale(dimensions);

	cube->SetRenderObject(new RenderObject(&cube->GetTransform(), cubeMesh, basicTex, basicShader));
	cube->SetPhysicsObject(new PhysicsObject(&cube->GetTransform(), cube->GetBoundingVolume()));

	cube->GetPhysicsObject()->SetInverseMass(inverseMass);
	cube->GetPhysicsObject()->InitCubeInertia();
	cube->GetTransform().SetLocalOrientation(q);
	cube->GetRenderObject()->SetColour(colour);
	world->AddGameObject(cube);

	return cube;
}

GameObject* TutorialGame::AddLakeToWorld(const Vector3& position, Vector3 dimensions, float inverseMass) {
	GameObject* cube = new GameObject("Lake");

	//Allows the goose to sink into the lake
	Vector3 dims = dimensions;
	dims.y -= 0.5;

	AABBVolume* volume = new AABBVolume(dims);
	cube->SetBoundingVolume((CollisionVolume*)volume);

	cube->GetTransform().SetWorldPosition(position);
	cube->GetTransform().SetWorldScale(dimensions);

	cube->SetRenderObject(new RenderObject(&cube->GetTransform(), cubeMesh, basicTex, basicShader));
	cube->SetPhysicsObject(new PhysicsObject(&cube->GetTransform(), cube->GetBoundingVolume()));

	cube->GetPhysicsObject()->SetInverseMass(inverseMass);
	cube->GetPhysicsObject()->InitCubeInertia();

	world->AddGameObject(cube);
	cube->GetRenderObject()->SetColour(Vector4(0, 0, 1, 0));
	return cube;
}

GameObject* TutorialGame::AddIslandToWorld() {
	GameObject* cube = new GameObject("Island");

	AABBVolume* volume = new AABBVolume(Vector3(50, 3, 20));

	cube->SetBoundingVolume((CollisionVolume*)volume);

	cube->GetTransform().SetWorldPosition(Vector3(-300, -2, 0));
	cube->GetTransform().SetWorldScale(Vector3(50, 3, 20));

	cube->SetRenderObject(new RenderObject(&cube->GetTransform(), cubeMesh, basicTex, basicShader));
	cube->SetPhysicsObject(new PhysicsObject(&cube->GetTransform(), cube->GetBoundingVolume()));

	cube->GetPhysicsObject()->SetInverseMass(0);
	cube->GetPhysicsObject()->InitCubeInertia();
	cube->GetRenderObject()->SetColour(Vector4(0.55, 0.71, 0, 1));
	world->AddGameObject(cube);


	return cube;
}

Player* TutorialGame::AddGooseToWorld(const Vector3& position)
{
	float size			= 1.0f;
	float inverseMass	= 1.0f;

	Player* goose = new Player("Dylan");


	SphereVolume* volume = new SphereVolume(size);
	goose->SetBoundingVolume((CollisionVolume*)volume);

	goose->GetTransform().SetWorldScale(Vector3(size,size,size) );
	goose->GetTransform().SetWorldPosition(position);

	goose->SetRenderObject(new RenderObject(&goose->GetTransform(), gooseMesh, nullptr, basicShader));
	goose->SetPhysicsObject(new PhysicsObject(&goose->GetTransform(), goose->GetBoundingVolume()));

	goose->GetPhysicsObject()->SetInverseMass(inverseMass);
	goose->GetPhysicsObject()->InitSphereInertia();

	world->AddGameObject(goose);

	return goose;
}

GameObject* TutorialGame::AddParkKeeperToWorld(const Vector3& position)
{
	float meshSize = 4.0f;
	float inverseMass = 0.5f;

	GameObject* keeper = new GameObject("Percy");

	AABBVolume* volume = new AABBVolume(Vector3(0.3, 0.9f, 0.3) * meshSize);
	keeper->SetBoundingVolume((CollisionVolume*)volume);

	keeper->GetTransform().SetWorldScale(Vector3(meshSize, meshSize, meshSize));
	keeper->GetTransform().SetWorldPosition(position);

	keeper->SetRenderObject(new RenderObject(&keeper->GetTransform(), keeperMesh, nullptr, basicShader));
	keeper->SetPhysicsObject(new PhysicsObject(&keeper->GetTransform(), keeper->GetBoundingVolume()));

	keeper->GetPhysicsObject()->SetInverseMass(inverseMass);
	keeper->GetPhysicsObject()->InitCubeInertia();

	world->AddGameObject(keeper);

	return keeper;
}

Enemy* TutorialGame::AddCharacterToWorld(const Vector3& position) {
	float meshSize = 4.0f;
	float inverseMass = 0.5f;

	auto pos = keeperMesh->GetPositionData();

	Vector3 minVal = pos[0];
	Vector3 maxVal = pos[0];

	for (auto& i : pos) {
		maxVal.y = max(maxVal.y, i.y);
		minVal.y = min(minVal.y, i.y);
	}

	Enemy* character = new Enemy("William", player);

	float r = rand() / (float)RAND_MAX;


	AABBVolume* volume = new AABBVolume(Vector3(0.3, 0.9f, 0.3) * meshSize);
	character->SetBoundingVolume((CollisionVolume*)volume);

	character->GetTransform().SetWorldScale(Vector3(meshSize, meshSize, meshSize));
	character->GetTransform().SetWorldPosition(position);
	character->setStartingPosition(position);

	character->SetRenderObject(new RenderObject(&character->GetTransform(), r > 0.5f ? charA : charB, nullptr, basicShader));
	character->SetPhysicsObject(new PhysicsObject(&character->GetTransform(), character->GetBoundingVolume()));

	character->GetPhysicsObject()->SetInverseMass(inverseMass);
	character->GetPhysicsObject()->InitCubeInertia();

	world->AddGameObject(character);

	return character;
}

Collectable* TutorialGame::AddAppleToWorld(const Vector3& position) {
	Collectable* apple = new Collectable("Apple");
	
	SphereVolume* volume = new SphereVolume(0.7f);
	apple->SetBoundingVolume((CollisionVolume*)volume);
	apple->GetTransform().SetWorldScale(Vector3(4, 4, 4));
	apple->GetTransform().SetWorldPosition(position);

	apple->SetRenderObject(new RenderObject(&apple->GetTransform(), appleMesh, nullptr, basicShader));
	apple->SetPhysicsObject(new PhysicsObject(&apple->GetTransform(), apple->GetBoundingVolume()));
	apple->setStartMass(1.0f);
	apple->GetPhysicsObject()->SetInverseMass(1.0f);
	apple->GetPhysicsObject()->InitSphereInertia();

	world->AddGameObject(apple);

	return apple;
}

Collectable* TutorialGame::AddBonusToWorld(const Vector3& position, Vector3 dimensions, float inverseMass) {
	Collectable* bonus = new Collectable("Bonus Cube", true, 500);
	bonus->setStartingPosition(position);
	bonus->setStartMass(inverseMass);
	AABBVolume* volume = new AABBVolume(dimensions);

	bonus->SetBoundingVolume((CollisionVolume*)volume);

	bonus->GetTransform().SetWorldPosition(position);
	bonus->GetTransform().SetWorldScale(dimensions);

	bonus->SetRenderObject(new RenderObject(&bonus->GetTransform(), cubeMesh, blankTex, basicShader));
	bonus->SetPhysicsObject(new PhysicsObject(&bonus->GetTransform(), bonus->GetBoundingVolume()));

	bonus->GetPhysicsObject()->SetInverseMass(inverseMass);
	bonus->GetPhysicsObject()->InitCubeInertia();

	bonus->GetRenderObject()->SetColour(Vector4(0.82f, 0.7f, 0.23f, 1));
	world->AddGameObject(bonus);

	return bonus;
}

void TutorialGame::InitSphereGridWorld(int numRows, int numCols, float rowSpacing, float colSpacing, float radius) {
	for (int x = 0; x < numCols; ++x) {
		for (int z = 0; z < numRows; ++z) {
			Vector3 position = Vector3(x * colSpacing, 10.0f, z * rowSpacing);
			AddSphereToWorld(position, radius, 1.0f);
		}
	}
}

void TutorialGame::InitMixedGridWorld(int numRows, int numCols, float rowSpacing, float colSpacing) {
	float sphereRadius = 1.0f;
	Vector3 cubeDims = Vector3(1, 1, 1);

	for (int x = 0; x < numCols; ++x) {
		for (int z = 0; z < numRows; ++z) {
			Vector3 position = Vector3(x * colSpacing, 10.0f, z * rowSpacing);

			if (rand() % 2) {
				AddCubeToWorld(position, cubeDims);
			}
			else {
				AddSphereToWorld(position, sphereRadius);
			}
		}
	}
}

void TutorialGame::InitCubeGridWorld(int numRows, int numCols, float rowSpacing, float colSpacing, const Vector3& cubeDims) {
	for (int x = 1; x < numCols+1; ++x) {
		for (int z = 1; z < numRows+1; ++z) {
			Vector3 position = Vector3(x * colSpacing, 10.0f, z * rowSpacing);
			AddCubeToWorld(position, cubeDims, "cube", 1.0f);
		}
	}
}

void TutorialGame::BridgeConstraintTest(Vector3 startPos) {
	Vector3 cubeSize = Vector3(5, 1, 1);

	float	invCubeMass = 0.1; //how heavy middle pieces are
	int		numLinks	= 16;
	float	maxDistance	= 2.05; //constraint distance
	float	cubeDistance = 2.055555556; // distance between links

	GameObject* start = AddCubeToWorld(startPos, cubeSize, "Bridge Start", 0, Vector4(0.93, 0.91, 0.67, 1));

	GameObject* end = AddCubeToWorld(startPos - Vector3(0, 0, (numLinks + 2) * cubeDistance), cubeSize, "Bridge End", 0, Vector4(0.93, 0.91, 0.67, 1));

	GameObject* previous = start;

	for (int i = 0; i < numLinks; ++i) {
		GameObject* block = AddCubeToWorld(startPos - Vector3(0, 0, (i + 1) * cubeDistance), cubeSize, "Bridge Strut", invCubeMass, Vector4(0.93, 0.91, 0.67, 1));
		PositionConstraint* constraint = new PositionConstraint(previous, block, maxDistance);
		world->AddConstraint(constraint);
		previous = block;
	}

	PositionConstraint* constraint = new PositionConstraint(previous, end, maxDistance);
	world->AddConstraint(constraint);
}

void TutorialGame::SimpleGJKTest() {
	Vector3 dimensions		= Vector3(5, 5, 5);
	Vector3 floorDimensions = Vector3(100, 2, 100);

	GameObject* fallingCube = AddCubeToWorld(Vector3(0, 20, 0), dimensions, "cube", 10.0f);
	GameObject* newFloor	= AddCubeToWorld(Vector3(0, 0, 0), floorDimensions,"cube", 0.0f);

	delete fallingCube->GetBoundingVolume();
	delete newFloor->GetBoundingVolume();

	fallingCube->SetBoundingVolume((CollisionVolume*)new OBBVolume(dimensions));
	newFloor->SetBoundingVolume((CollisionVolume*)new OBBVolume(floorDimensions));

}

