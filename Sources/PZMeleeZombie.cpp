//
//  PZMeleeZombie.cpp
//  Project: Z
//
//  Copyright 2018 by Überpixel. All rights reserved.
//  Unauthorized use is punishable by torture, mutilation, and vivisection.
//

#include "PZMeleeZombie.h"
#include "PZWorld.h"

namespace PZ
{
	MeleeZombie::MeleeZombie() : Zombie(RN::Model::WithName(RNCSTR("models/zombies/zombie2.sgm")))
	{
		_attackSounds = new RN::Array();
		_attackSounds->AddObject(RN::AudioAsset::WithName(RNCSTR("audio/melee1.ogg")));
		_attackSounds->AddObject(RN::AudioAsset::WithName(RNCSTR("audio/melee2.ogg")));
		_attackSounds->AddObject(RN::AudioAsset::WithName(RNCSTR("audio/melee3.ogg")));
		
		_idleSounds = new RN::Array();
		_idleSounds->AddObject(RN::AudioAsset::WithName(RNCSTR("audio/idle1.ogg")));
		_idleSounds->AddObject(RN::AudioAsset::WithName(RNCSTR("audio/idle2.ogg")));
		_idleSounds->AddObject(RN::AudioAsset::WithName(RNCSTR("audio/idle3.ogg")));
		
		_mouthSource = new RN::SteamAudioSource(nullptr, false);
		_mouthSource->SetTimeOfFlight(false);
		_mouthSource->SetRadius(0.0f);
		_mouthSource->SetGain(0.5f);
		AddChild(_mouthSource->Autorelease());
		_mouthSource->SetPosition(RN::Vector3(0.0f, 1.65f, -0.1f));
		
		_idleTimer = RN::RandomNumberGenerator::GetSharedGenerator()->GetRandomFloatRange(0, 20);
	}

	MeleeZombie::~MeleeZombie()
	{
		_attackSounds->Release();
		_idleSounds->Release();
	}

	void MeleeZombie::Update(float delta)
	{
		Zombie::Update(delta);

		PZ::Player *player = World::GetSharedInstance()->GetPlayer();
		RN::Vector3 vec = player->GetPosition() - GetPosition();
		vec.y = 0;
		if (vec.GetLength() < 1.0f)
		{
			if(!_mouthSource->IsPlaying() || _mouthSource->HasEnded())
			{
				int attackSoundIndex = RN::RandomNumberGenerator::GetSharedGenerator()->GetRandomInt32Range(0, _attackSounds->GetCount());
				_mouthSource->SetAudioAsset(_attackSounds->GetObjectAtIndex<RN::AudioAsset>(attackSoundIndex));
				_mouthSource->Seek(0.0);
				_mouthSource->Play();
			}
			player->Die();
		}
		
		if(!_following)
		{
			_idleTimer -= delta;
			if(_idleTimer < 0.0 && (_mouthSource->IsPlaying() || _mouthSource->HasEnded()))
			{
				_idleTimer = RN::RandomNumberGenerator::GetSharedGenerator()->GetRandomFloatRange(10, 20);
				int idleSoundIndex = RN::RandomNumberGenerator::GetSharedGenerator()->GetRandomInt32Range(0, _idleSounds->GetCount());
				_mouthSource->SetAudioAsset(_idleSounds->GetObjectAtIndex<RN::AudioAsset>(idleSoundIndex));
				_mouthSource->Seek(0.0);
				_mouthSource->Play();
			}
		}
	}
}
