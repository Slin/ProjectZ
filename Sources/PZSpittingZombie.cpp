//
//  PZSpittingZombie.cpp
//  Project: Z
//
//  Copyright 2018 by Überpixel. All rights reserved.
//  Unauthorized use is punishable by torture, mutilation, and vivisection.
//

#include "PZSpittingZombie.h"
#include "PZWorld.h"

namespace PZ
{
	SpittingZombie::SpittingZombie()
	{
		RN::Model *model = RN::Model::WithName(RNCSTR("models/zombies/spit.sgm"));
		for (int i = 0; i < PZ_MAX_SPIT_BLOBS; i++) {
			RN::Entity *entity = new RN::Entity(model);
			_spitBlobs[i].entity = entity;
			_spitBlobs[i].active = false;
		}
		_spitBlobIndex = 0;
		_spitNextTime = 0;
	}

	SpittingZombie::~SpittingZombie()
	{
		//		SafeRelease(_controller);
	}

	void SpittingZombie::Update(float delta)
	{
		ZombieBase::Update(delta);

		bool anyActive = false;
		for (int i = 0; i < PZ_MAX_SPIT_BLOBS; i++) {
			SpitBlob *blob = &_spitBlobs[i];
			if (!blob->active) {
				continue;
			}
			anyActive = true;
			blob->velocity.y -= 9.8 * delta;
			RN::Vector3 pos = blob->entity->GetPosition();
			blob->entity->SetPosition(pos + blob->velocity * delta);
		}

		PZ::Player *player = World::GetSharedInstance()->GetPlayer();
		float dist = (player->GetPosition() - GetPosition()).GetLength();
		bool spitting = dist < 3;

		if (!anyActive && !spitting) {
			_spitNextTime = 0.5f;
			return;
		}

		_spitNextTime -= delta;
		while (_spitNextTime <= 0) {
			_spitNextTime += 0.04f;
			SpitBlob *blob = &_spitBlobs[_spitBlobIndex];
			if (spitting) {
				if (!blob->active) {
					World::GetSharedInstance()->AddNode(blob->entity);
					blob->active = true;
				}
				RN::Vector3 spitPos = GetPosition();
				spitPos.y = 1.2;
				blob->entity->SetPosition(spitPos);

				RN::Vector3 targetPos = player->GetWorldPosition();
				targetPos.y = 2.4;

				blob->velocity = (targetPos - spitPos).GetNormalized(8.0f);
				float r = float(rand()) / float(RAND_MAX);
				blob->velocity.x += (r * 2.0f - 1.0f) * 1.8f;
				r = float(rand()) / float(RAND_MAX);
				blob->velocity.y += (r * 2.0f - 1.0f) * 1.8f;
				r = float(rand()) / float(RAND_MAX);
				blob->velocity.z += (r * 2.0f - 1.0f) * 1.8f;
			}
			else if (blob->active) {
				blob->active = false;
				World::GetSharedInstance()->RemoveNode(blob->entity);
			}
			_spitBlobIndex++;
			_spitBlobIndex %= PZ_MAX_SPIT_BLOBS;
		}
	}
}
