//
//  PZZombieBase.cpp
//  Project: Z
//
//  Copyright 2018 by Überpixel. All rights reserved.
//  Unauthorized use is punishable by torture, mutilation, and vivisection.
//

#include "PZZombieBase.h"
#include "PZWorld.h"

namespace PZ
{
	ZombieBase::ZombieBase()
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

		RN::Model *model = RN::Model::WithName(RNCSTR("models/zombies/zombie1.sgm"));
		_zombie = new RN::Entity(model);
		AddChild(_zombie->Autorelease());

		_following = false;
		_storeSpawnPoint = true;
		_returnToSpawn = false;
	}

	ZombieBase::~ZombieBase()
	{
		//		SafeRelease(_controller);
	}

	void ZombieBase::Update(float delta)
	{
		if (_storeSpawnPoint) {
			_spawnPoint = GetWorldPosition();
			_storeSpawnPoint = false;
		}

		RN::SceneNode::Update(delta);

		bool playerIsDead = World::GetSharedInstance()->GetPlayer()->IsDead();
		if (playerIsDead) {
			_returnToSpawn = true;
		}

		bool canSeePlayer = World::GetSharedInstance()->IsPlayerVisibleFrom(GetWorldPosition() + RN::Vector3(0, 1, 0));

		if (!playerIsDead && canSeePlayer && _returnToSpawn) {
			_returnToSpawn = false;
		} else if (_returnToSpawn) {
			//SetWorldPosition(_spawnPoint);
			//_returnToSpawn = false;

			_navigationAgent->SetTarget(_spawnPoint, RN::Vector3(20.0f));
			if ((GetWorldPosition() - _spawnPoint).GetLength() < 2.0f) {
				_returnToSpawn = false;
			}
			return;
		}

		if (canSeePlayer) {
			_following = true;
			_followTime = 1.5f;
		}
		else if (_followTime > 0) {
			_followTime -= delta;
			_following = _followTime > 0;
		}

		if (_following) {
			_navigationAgent->SetTarget(World::GetSharedInstance()->GetPlayer()->GetWorldPosition(), RN::Vector3(5.0f));
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
