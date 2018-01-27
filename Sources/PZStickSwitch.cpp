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
