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
		
		_raycastOffset.z = -0.1f;
	}
	
	ButtonSwitch::~ButtonSwitch()
	{
		
	}
	
	void ButtonSwitch::Update(float delta)
	{
		Switch::Update(delta);
		
		if(_isActive)
		{
			float diff = 0.03 - _button->GetPosition().z;
			if(diff < RN::k::EpsilonFloat)
				Switch::SetActive(false);
			
			if(diff > delta*0.1)
				diff = delta*0.1;
			
			_button->SetPosition(_button->GetPosition() + RN::Vector3(0.0f, 0.0f, diff));
		}
		else
		{
			float diff = 0.0 - _button->GetPosition().z;
			if(diff < -delta*0.1)
				diff = -delta*0.1;
			
			_button->SetPosition(_button->GetPosition() + RN::Vector3(0.0f, 0.0f, diff));
		}
	}
	
	void ButtonSwitch::SetActive(bool active)
	{
		if(active)
			Switch::SetActive(active);
	}
}
