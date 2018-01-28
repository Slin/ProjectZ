//
//  PZStickSwitch.cpp
//  Project: Z
//
//  Copyright 2018 by Überpixel. All rights reserved.
//  Unauthorized use is punishable by torture, mutilation, and vivisection.
//

#include "PZStickSwitch.h"
#include "PZWorld.h"

namespace PZ
{
	StickSwitch::StickSwitch()
	{
		_base = new RN::Entity(RN::Model::WithName(RNCSTR("models/objects/stick_switch_base.sgm")));
		AddChild(_base->Autorelease());
		
		_stick = new RN::Entity(RN::Model::WithName(RNCSTR("models/objects/stick_switch_stick.sgm")));
		AddChild(_stick->Autorelease());
		
		_stick->SetRotation(RN::Vector3(0.0f, 0.0f, 90.0f));
		_raycastOffset.x = -0.1f;
		
		RN::AudioAsset *audioAsset = RN::AudioAsset::WithName(RNCSTR("audio/switch.ogg"));
		_switchSource = new RN::SteamAudioSource(audioAsset, false);
		_switchSource->SetTimeOfFlight(false);
		_switchSource->SetRepeat(false);
		_switchSource->SetRadius(0.0f);
		AddChild(_switchSource->Autorelease());
		_switchSource->SetPosition(RN::Vector3(0.0f, 0.0f, -0.03f));
	}
	
	StickSwitch::~StickSwitch()
	{
		
	}
	
	void StickSwitch::Update(float delta)
	{
		Switch::Update(delta);
		
		if(_isActive)
		{
			float diff = 0.0f - _stick->GetRotation().GetEulerAngle().z;
			if(diff < -delta*70.0f)
				diff = -delta*70.0f;
			
			_stick->SetRotation(_stick->GetRotation() + RN::Vector3(0.0f, 0.0f, diff));
		}
		else
		{
			float diff = 90.0 - _stick->GetRotation().GetEulerAngle().z;
			if(diff > delta*70.0f)
				diff = delta*70.0f;
			
			_stick->SetRotation(_stick->GetRotation() + RN::Vector3(0.0f, 0.0f, diff));
		}
	}
	
	void StickSwitch::SetActive(bool active)
	{
		if(_switchSource->HasEnded() || !_switchSource->IsPlaying())
		{
			_switchSource->Seek(0.0f);
			_switchSource->Play();
			Switch::SetActive(active);
		}
	}
}
