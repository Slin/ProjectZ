//
//  PZPlayer.cpp
//  Project: Z
//
//  Copyright 2018 by Überpixel. All rights reserved.
//  Unauthorized use is punishable by torture, mutilation, and vivisection.
//

#include "PZPlayer.h"
#include "PZWorld.h"

namespace PZ
{
	Player::Player()
	{
		
	}
	
	Player::~Player()
	{
		//TODO: Cleanup
	}
	
	void Player::Update(float delta)
	{
		RN::SceneNode::Update(delta);
	}
}
