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
	}
	
	Zombie::~Zombie()
	{
//		SafeRelease(_controller);
	}
	
	void Zombie::Update(float delta)
	{
		RN::SceneNode::Update(delta);
		
		_navigationAgent->SetTarget(World::GetSharedInstance()->GetPlayer()->GetWorldPosition(), RN::Vector3(5.0f));
		
		RN::Vector3 lookDir = GetWorldPosition()-_previousPosition;
		if(lookDir.GetLength() > 0.01f)
		{
			RN::Quaternion lookatRotation = RN::Quaternion::WithLookAt(-lookDir);
			_zombie->SetWorldRotation(lookatRotation);
		}
		_previousPosition = GetWorldPosition();
	}
}
