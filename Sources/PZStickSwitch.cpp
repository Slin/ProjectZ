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
	}
	
	StickSwitch::~StickSwitch()
	{
		
	}
	
	void StickSwitch::Update(float delta)
	{
		Switch::Update(delta);
	}
	
	void StickSwitch::SetActive(bool active)
	{
		Switch::SetActive(active);
	}
}
