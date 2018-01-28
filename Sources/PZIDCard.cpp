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
	}
	
	IDCard::~IDCard()
	{
//		SafeRelease(_controller);
	}
	
	void IDCard::Update(float delta)
	{
		if (_entity == nullptr) {
			return;
		}

		Player *player = World::GetSharedInstance()->GetPlayer();
		RN::Vector3 vec = player->GetWorldPosition() - GetWorldPosition();
		vec.y = 0;
		if (vec.GetLength() < 0.7f) {
			RemoveChild(_entity);
			_entity = nullptr;

			player->GiveIDCard();
		}
	}
}
