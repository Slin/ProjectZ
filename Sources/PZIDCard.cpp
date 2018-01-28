//
//  PZIDCard.cpp
//  Project: Z
//
//  Copyright 2018 by Überpixel. All rights reserved.
//  Unauthorized use is punishable by torture, mutilation, and vivisection.
//

#include "PZIDCard.h"
#include "PZWorld.h"

namespace PZ
{
	IDCard::IDCard()
	{
		_entity = new RN::Entity(RN::Model::WithName(RNCSTR("models/objects/id_card.sgm")));
		AddChild(_entity->Autorelease());
		_taken = false;
	}
	
	IDCard::~IDCard()
	{
//		SafeRelease(_controller);
	}
	
	void IDCard::Update(float delta)
	{
		Player *player = World::GetSharedInstance()->GetPlayer();

		if (_taken) {
			if (player->IsDead()) {
				_entity->SetFlags(0);
				_taken = false;
			}
			return;
		}

		RN::Vector3 vec = player->GetWorldPosition() - GetWorldPosition();
		vec.y = 0;
		if (vec.GetLength() < 0.7f) {
			_entity->SetFlags(RN::SceneNode::Flags::Hidden);
			player->GiveIDCard();
			_taken = true;
		}
	}
}
