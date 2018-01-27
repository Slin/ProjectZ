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
		_navigationAgent = new RN::RecastAgent();
		AddAttachment(_navigationAgent);
		
		RN::Model *model = RN::Model::WithName(RNCSTR("models/zombies/testzombie.sgm"));
		RN::Entity *entity = new RN::Entity(model);
		AddChild(entity->Autorelease());
		entity->SetPosition(RN::Vector3(0.0, 1.0, 0.0));
	}
	
	Zombie::~Zombie()
	{
//		SafeRelease(_controller);
	}
	
	void Zombie::Update(float delta)
	{
		RN::SceneNode::Update(delta);
		
		_navigationAgent->SetTarget(World::GetSharedInstance()->GetPlayer()->GetWorldPosition(), RN::Vector3(5.0f));
	}
}
