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
		/*		RN::PhysXMaterial *physicsMaterial = new RN::PhysXMaterial();
		_controller = new RN::PhysXKinematicController(0.25f, 1.1f, physicsMaterial->Autorelease());
		_controller->SetPositionOffset(_controller->GetFeetOffset());
		_controller->SetCollisionFilter(World::CollisionType::Character, World::CollisionType::Level | World::CollisionType::Character);
		AddAttachment(_controller);*/

		SetWorldPosition(RN::Vector3(17.0, 0.0, 0.0));
		RN::RecastAgent::Settings settings;
		_navigationAgent = new RN::RecastAgent(settings);
		AddAttachment(_navigationAgent);

		RN::Model *model = RN::Model::WithName(RNCSTR("models/zombies/zombie1.sgm"));
		_zombie = new RN::Entity(model);
		AddChild(_zombie->Autorelease());

		model = RN::Model::WithName(RNCSTR("models/zombies/spit.sgm"));
		for (int i = 0; i < PZ_MAX_SPIT_BLOBS; i++) {
			RN::Entity *entity = new RN::Entity(model);
			_spitBlobs[i].entity = entity;
			_spitBlobs[i].active = false;
		}
		_spitBlobIndex = 0;
		_spitNextTime = 0;
	}

	Zombie::~Zombie()
	{
		//		SafeRelease(_controller);
	}

	void Zombie::Update(float delta)
	{
		RN::SceneNode::Update(delta);

		_navigationAgent->SetTarget(World::GetSharedInstance()->GetPlayer()->GetWorldPosition(), RN::Vector3(5.0f));

		RN::Vector3 lookDir = GetWorldPosition() - _previousPosition;
		if (lookDir.GetLength() > 0.01f)
		{
			RN::Quaternion lookatRotation = RN::Quaternion::WithLookAt(-lookDir);
			_zombie->SetWorldRotation(lookatRotation);
		}
		_previousPosition = GetWorldPosition();

		RunSpit(delta);
	}

	bool Zombie::ShouldSpit() {
		PZ::Player *player = World::GetSharedInstance()->GetPlayer();
		float dist = (player->GetPosition() - GetPosition()).GetLength();
		return dist < 5;
	}

	void Zombie::RunSpit(float delta) {
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

		bool spitting = ShouldSpit();
		if (!anyActive && !spitting) {
			_spitNextTime = 0;
			return;
		}

		PZ::Player *player = World::GetSharedInstance()->GetPlayer();

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
