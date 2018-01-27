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

		if (_following) {
			RN::Vector3 playerPos = player->GetWorldPosition();
			RN::Vector3 zombiePos = GetWorldPosition();
			playerPos.y = 0;
			zombiePos.y = 0;
			if ((playerPos - zombiePos).GetLength() > 0.9f) {
				_navigationAgent->SetTarget(player->GetWorldPosition());
			}
			else {
				_navigationAgent->SetTarget(GetWorldPosition());
			}
		}
		else {
			_navigationAgent->SetTarget(GetWorldPosition());
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
