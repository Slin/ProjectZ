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
	Switch::Switch() : _isActive(false)
	{
		
	}
	
	Switch::~Switch()
	{
//		SafeRelease(_controller);
	}
	
	void Switch::Update(float delta)
	{
		RN::SceneNode::Update(delta);
		
		float distanceToPlayer = (World::GetSharedInstance()->GetPlayer()->GetWorldPosition()-GetWorldPosition()).GetLength();
		if(distanceToPlayer < 2.0f)
		{
			if(World::GetSharedInstance()->GetPlayer()->IsActivatePressed())
			{
				if(!_isKeyPressed)
					SetActive(!_isActive);
				
				_isKeyPressed = true;
			}
			else
			{
				_isKeyPressed = false;
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
