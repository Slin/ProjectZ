//
//  PZZombie.cpp
//  Project: Z
//
//  Copyright 2018 by Überpixel. All rights reserved.
//  Unauthorized use is punishable by torture, mutilation, and vivisection.
//

#include "PZZombie.h"
#include "PZWorld.h"

namespace PZ
{
	Zombie::Zombie(RN::Model *model)
	{
		/*		RN::PhysXMaterial *physicsMaterial = new RN::PhysXMaterial();
		_controller = new RN::PhysXKinematicController(0.25f, 1.1f, physicsMaterial->Autorelease());
		_controller->SetPositionOffset(_controller->GetFeetOffset());
		_controller->SetCollisionFilter(World::CollisionType::Character, World::CollisionType::Level | World::CollisionType::Character);
		AddAttachment(_controller);*/

		SetWorldPosition(RN::Vector3(17.0, 0.0, 0.0));
		RN::RecastAgent::Settings settings;
		_navigationAgent = new RN::RecastAgent(settings);
		AddAttachment(_navigationAgent);

		_zombie = new RN::Entity(model);
		AddChild(_zombie->Autorelease());

		_following = false;
		_storeSpawnPoint = true;
		_returnToSpawn = false;
	}

	Zombie::~Zombie()
	{
		//		SafeRelease(_controller);
	}

	void Zombie::Update(float delta)
	{
		if (_storeSpawnPoint) {
			_spawnPoint = GetWorldPosition();
			_storeSpawnPoint = false;
		}

		RN::SceneNode::Update(delta);

		bool playerIsDead = World::GetSharedInstance()->GetPlayer()->IsDead();
		if (playerIsDead) {
			_returnToSpawn = true;
		} else if (_returnToSpawn) {
			SetWorldPosition(_spawnPoint);
			_returnToSpawn = false;
			_followTime = 0;
			_following = false;
		}

		bool canSeePlayer = World::GetSharedInstance()->IsPlayerVisibleFrom(GetWorldPosition() + RN::Vector3(0, 1, 0));

		if (canSeePlayer) {
			_following = true;
			_followTime = 1.5f;
		}
		else if (_followTime > 0) {
			_followTime -= delta;
			_following = _followTime > 0;
		}
		
		Player *player = World::GetSharedInstance()->GetPlayer();

		if (_following && (player->GetWorldPosition() - GetWorldPosition()).GetLength() > 0.7f) {
			_navigationAgent->SetTarget(player->GetWorldPosition(), RN::Vector3(5.0f));
		}
		else {
			_navigationAgent->SetTarget(GetWorldPosition(), RN::Vector3(5.0f));
		}

		RN::Vector3 lookDir = GetWorldPosition() - _previousPosition;
		if (lookDir.GetLength() > 0.005f)
		{
			RN::Quaternion lookatRotation = RN::Quaternion::WithLookAt(-lookDir);
			_zombie->SetWorldRotation(lookatRotation);
		}

		_previousPosition = GetWorldPosition();
	}
}
