//
//  PZSwitch.cpp
//  Project: Z
//
//  Copyright 2018 by Überpixel. All rights reserved.
//  Unauthorized use is punishable by torture, mutilation, and vivisection.
//

#include "PZSwitch.h"
#include "PZWorld.h"

namespace PZ
{
	Switch::Switch() : _isActive(false), _needsReset(false), _needsKey(true)
	{
		RN::AudioAsset *audioAsset = RN::AudioAsset::WithName(RNCSTR("audio/locked.ogg"));
		_lockedSource = new RN::SteamAudioSource(audioAsset, false);
		_lockedSource->SetTimeOfFlight(false);
		_lockedSource->SetRepeat(false);
		_lockedSource->SetRadius(0.0f);
		AddChild(_lockedSource->Autorelease());
		_lockedSource->SetPosition(RN::Vector3(0.0f, 0.0f, -0.03f));
	}
	
	Switch::~Switch()
	{
//		SafeRelease(_controller);
	}
	
	void Switch::Update(float delta)
	{		
		RN::SceneNode::Update(delta);
		
		bool playerIsDead = World::GetSharedInstance()->GetPlayer()->IsDead();
		if(playerIsDead)
		{
			_needsReset = true;
		}
		else if(_needsReset)
		{
			_isActive = false;
			_needsReset = false;
		}
		
		RN::Vector3 from = GetWorldPosition()+GetWorldRotation().GetRotatedVector(_raycastOffset);
		RN::Vector3 to = World::GetSharedInstance()->GetPlayer()->GetWorldPosition() + RN::Vector3(0.0f, 1.0f, 0.0f);
		RN::Vector3 diff = to - from;
		float distanceToPlayer = diff.GetLength();
		if(distanceToPlayer < 1.0f)
		{
			RN::PhysXContactInfo hit;
			hit.distance = -1.0f;
			if(distanceToPlayer > 0.1f)
			{
				hit = World::GetSharedInstance()->GetPhysicsWorld()->CastRay(from, to, World::CollisionType::Level);
			}
			if(hit.distance < -0.5f)
			{
				if(World::GetSharedInstance()->GetPlayer()->IsActivatePressed())
				{
					if(!_isKeyPressed)
					{
						bool hasKey = World::GetSharedInstance()->GetPlayer()->HasIDCard();
						if(_needsKey && !hasKey)
						{
							if((!_lockedSource->IsPlaying() || _lockedSource->HasEnded()))
							{
								_lockedSource->SetGain(0.5f);
								_lockedSource->Seek(0.0);
								_lockedSource->Play();
							}
						}
						else
						{
							SetActive(!_isActive);
						}
					}
					
					_isKeyPressed = true;
				}
				else
				{
					_isKeyPressed = false;
				}
			}
		}
	}
	
	void Switch::SetActive(bool active)
	{
		_isActive = active;
		
		if(_action)
			_action(_isActive);
	}
	
	void Switch::SetAction(std::function< void(bool) > action)
	{
		_action = action;
	}
}
