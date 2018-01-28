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
	Zombie::Zombie(RN::Model *model, float normalSpeed, float followSpeed)
	{
		_normalSpeed = normalSpeed;
		_followSpeed = followSpeed;

		RN::RecastAgent::Settings settings;
		settings.maxSpeed = _normalSpeed;
		_navigationAgent = new RN::RecastAgent(settings);
		AddAttachment(_navigationAgent);

		_zombie = new RN::Entity(model);
		AddChild(_zombie->Autorelease());
		_zombie->SetPosition(RN::Vector3(0.0f, -0.15f, 0.0f));

		_following = false;
		_storeSpawnPoint = true;
		_returnToSpawn = false;
		_followTime = 0.0f;
		_slurpTime = 0.0f;
		
		RN::AudioAsset *audioAsset = RN::AudioAsset::WithName(RNCSTR("audio/slurp.ogg"));
		_slurpSource = new RN::SteamAudioSource(audioAsset, false);
		_slurpSource->SetTimeOfFlight(false);
		_slurpSource->Play();
		_slurpSource->SetRepeat(true);
		_slurpSource->SetRadius(0.0f);
		AddChild(_slurpSource->Autorelease());
		_slurpSource->SetPosition(RN::Vector3(0.0f, 0.0f, 0.0f));
	}

	Zombie::~Zombie()
	{
		
	}

	void Zombie::Update(float delta)
	{
		bool prevFollow = _following;

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
			_followTime = 1.0f;
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

		RN::Vector3 checkFrom = _previousPosition + RN::Vector3(0.0f, 1.0f, 0.0f);
		RN::Vector3 checkTo = GetWorldPosition() + RN::Vector3(0.0f, 1.0f, 0.0f);
		if ((checkFrom - checkTo).GetLength() > 0.001f) {
			RN::PhysXContactInfo hit = World::GetSharedInstance()->GetPhysicsWorld()->CastRay(checkFrom, checkTo);
			if (hit.distance > -0.5f)
			{
				_following = false;
				_followTime = 0;
			}
		}
		
		RN::Vector3 lookDir = GetWorldPosition() - _previousPosition;
		if (lookDir.GetLength() > 0.005f)
		{
			RN::Quaternion lookatRotation = RN::Quaternion::WithLookAt(-lookDir);
			_zombie->SetWorldRotation(lookatRotation);
		}
		
		if(_following)
		{
			_slurpTime += delta*2.0f;
			if(_slurpTime > 0.7f)
				_slurpTime = 0.7f;
		}
		else
		{
			_slurpTime -= delta*2.0f;
			if(_slurpTime < 0.0f)
				_slurpTime = 0.0f;
		}
		_slurpSource->SetGain(_slurpTime);

		_previousPosition = GetWorldPosition();

		if (_following != prevFollow) {
			RN::RecastAgent::Settings settings;
			settings.maxSpeed = _following ? _followSpeed : _normalSpeed;
			_navigationAgent->UpdateSettings(settings);
		}
	}
}
