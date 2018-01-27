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
	}

	MeleeZombie::~MeleeZombie()
	{
	}

	void MeleeZombie::Update(float delta)
	{
		Zombie::Update(delta);

		PZ::Player *player = World::GetSharedInstance()->GetPlayer();
		RN::Vector3 vec = player->GetPosition() - GetPosition();
		vec.y = 0;
		if (vec.GetLength() < 1.0f) {
			player->Die();
		}
	}
}
