// 
//  PZRangeZombie.cpp
//  Project: Z
//
//  Copyright 2018 by Überpixel. All rights reserved.
//  Unauthorized use is punishable by torture, mutilation, and vivisection.
//

#include "PZRangeZombie.h"
#include "PZWorld.h"

namespace PZ
{
	RangeZombie::RangeZombie() : Zombie(RN::Model::WithName(RNCSTR("models/zombies/zombie1.sgm")))
	{
		RN::Model *model = RN::Model::WithName(RNCSTR("models/zombies/spit.sgm"));
		for (int i = 0; i < PZ_MAX_SPIT_BLOBS; i++) {
			RN::Entity *entity = new RN::Entity(model);
			_spitBlobs[i].entity = entity;
			_spitBlobs[i].active = false;
		}
		_spitBlobIndex = 0;
		_spitNextTime = 0;
		
		_pukeSounds = new RN::Array();
		_pukeSounds->AddObject(RN::AudioAsset::WithName(RNCSTR("audio/pukeattack1.ogg")));
		_pukeSounds->AddObject(RN::AudioAsset::WithName(RNCSTR("audio/pukeattack2.ogg")));
		
		_idleSounds = new RN::Array();
		_idleSounds->AddObject(RN::AudioAsset::WithName(RNCSTR("audio/idle1.ogg")));
		_idleSounds->AddObject(RN::AudioAsset::WithName(RNCSTR("audio/idle2.ogg")));
		_idleSounds->AddObject(RN::AudioAsset::WithName(RNCSTR("audio/idle3.ogg")));
		
		_mouthSource = new RN::SteamAudioSource(nullptr, false);
		_mouthSource->SetTimeOfFlight(false);
		_mouthSource->SetRadius(0.0f);
		//_mouthSource->SetGain(0.1f);
		AddChild(_mouthSource->Autorelease());
		_mouthSource->SetPosition(RN::Vector3(0.0f, 1.65f, -0.1f));
		
		_idleTimer = RN::RandomNumberGenerator::GetSharedGenerator()->GetRandomFloatRange(0, 20);
	}

	RangeZombie::~RangeZombie()
	{
		_pukeSounds->Release();
		_idleSounds->Release();
	}

	void RangeZombie::Update(float delta)
	{
		Zombie::Update(delta);

		PZ::Player *player = World::GetSharedInstance()->GetPlayer();

		bool anyActive = false;
		for (int i = 0; i < PZ_MAX_SPIT_BLOBS; i++) {
			SpitBlob *blob = &_spitBlobs[i];
			if (!blob->active) {
				continue;
			}
			anyActive = true;
			blob->velocity.y -= 9.8 * delta;
			RN::Vector3 pos = blob->entity->GetPosition();
			blob->entity->SetPosition(pos + blob->velocity * delta);

			if (!player->IsDead()) {
				RN::Vector3 vec = player->GetPosition() - blob->entity->GetPosition();
				vec.y = 0;
				if (vec.GetLength() < 0.5f) {
					player->Die();
				}
			}
		}

		bool spitting = !player->IsDead();
		if (spitting) {
			float dist = (player->GetPosition() - GetPosition()).GetLength();
			spitting = dist < 3.0f;
		}
		if (spitting) {
			spitting = World::GetSharedInstance()->IsPlayerVisibleFrom(GetWorldPosition());
			
			if(!_mouthSource->IsPlaying() || _mouthSource->HasEnded())
			{
				int pukeSoundIndex = RN::RandomNumberGenerator::GetSharedGenerator()->GetRandomInt32Range(0, _pukeSounds->GetCount());
				_mouthSource->SetAudioAsset(_pukeSounds->GetObjectAtIndex<RN::AudioAsset>(pukeSoundIndex));
				_mouthSource->SetGain(1.0f);
				_mouthSource->Seek(0.0);
				_mouthSource->Play();
			}
		}
		if (!anyActive && !spitting) {
			_spitNextTime = 0.5f;
			return;
		}

		_spitNextTime -= delta;
		while (_spitNextTime <= 0) {
			_spitNextTime += 0.04f;
			SpitBlob *blob = &_spitBlobs[_spitBlobIndex];
			if (spitting) {
				if (!blob->active) {
					World::GetSharedInstance()->AddNode(blob->entity);
					blob->active = true;
				}
				RN::Vector3 spitPos = GetPosition();
				spitPos.y = 1.2f;
				blob->entity->SetPosition(spitPos);

				RN::Vector3 targetPos = player->GetWorldPosition();
				targetPos.y = 2.4f;

				blob->velocity = (targetPos - spitPos).GetNormalized(8.0f);
				float r = float(rand()) / float(RAND_MAX);
				blob->velocity.x += (r * 2.0f - 1.0f) * 1.8f;
				r = float(rand()) / float(RAND_MAX);
				blob->velocity.y += (r * 2.0f - 1.0f) * 1.8f;
				r = float(rand()) / float(RAND_MAX);
				blob->velocity.z += (r * 2.0f - 1.0f) * 1.8f;
			}
			else if (blob->active) {
				blob->active = false;
				World::GetSharedInstance()->RemoveNode(blob->entity);
			}
			_spitBlobIndex++;
			_spitBlobIndex %= PZ_MAX_SPIT_BLOBS;
		}
		
		if(!_following)
		{
			_idleTimer -= delta;
			if(_idleTimer < 0.0 && (!_mouthSource->IsPlaying() || _mouthSource->HasEnded()))
			{
				_idleTimer = RN::RandomNumberGenerator::GetSharedGenerator()->GetRandomFloatRange(10, 20);
				int idleSoundIndex = RN::RandomNumberGenerator::GetSharedGenerator()->GetRandomInt32Range(0, _idleSounds->GetCount());
				_mouthSource->SetAudioAsset(_idleSounds->GetObjectAtIndex<RN::AudioAsset>(idleSoundIndex));
				_mouthSource->SetGain(0.5f);
				_mouthSource->Seek(0.0);
				_mouthSource->Play();
			}
		}
	}
}
