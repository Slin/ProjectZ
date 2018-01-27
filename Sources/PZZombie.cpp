//
//  PZZombie.cpp
//  Project: Z
//
//  Copyright 2018 by Überpixel. All rights reserved.
//  Unauthorized use is punishable by torture, mutilation, and vivisection.
//

#include "PZZombie.h"
#include "PZWorld.h"

namespace PZ
{
	Zombie::Zombie()
	{
	}

	Zombie::~Zombie()
	{
	}

	void Zombie::Update(float delta)
	{
		ZombieBase::Update(delta);

		PZ::Player *player = World::GetSharedInstance()->GetPlayer();
		RN::Vector3 vec = player->GetPosition() - GetPosition();
		vec.y = 0;
		if (vec.GetLength() < 1.0f) {
			player->Die();
		}
	}
}
