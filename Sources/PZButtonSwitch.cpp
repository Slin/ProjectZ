//
//  PZButtonSwitch.cpp
//  Project: Z
//
//  Copyright 2018 by Überpixel. All rights reserved.
//  Unauthorized use is punishable by torture, mutilation, and vivisection.
//

#include "PZButtonSwitch.h"
#include "PZWorld.h"

namespace PZ
{
	ButtonSwitch::ButtonSwitch()
	{
		_base = new RN::Entity(RN::Model::WithName(RNCSTR("models/objects/button_switch_base.sgm")));
		AddChild(_base->Autorelease());
		
		_button = new RN::Entity(RN::Model::WithName(RNCSTR("models/objects/button_switch_button.sgm")));
		AddChild(_button->Autorelease());
	}
	
	ButtonSwitch::~ButtonSwitch()
	{
		
	}
	
	void ButtonSwitch::Update(float delta)
	{
		Switch::Update(delta);
	}
	
	void ButtonSwitch::SetActive(bool active)
	{
		Switch::SetActive(active);
	}
}
