//
//  PZDoor.cpp
//  Project: Z
//
//  Copyright 2018 by Überpixel. All rights reserved.
//  Unauthorized use is punishable by torture, mutilation, and vivisection.
//

#include "PZDoor.h"
#include "PZWorld.h"

namespace PZ
{
	Door::Door(RN::String *filename, RN::Vector3 openOffset) : _isActive(true), _openOffset(openOffset)
	{
		RN::Model *model = RN::Model::WithName(filename);
		_door = new RN::Entity(model);
		AddChild(_door->Autorelease());
		
		RN::PhysXMaterial *physicsMaterial = new RN::PhysXMaterial();
		_body = new RN::PhysXDynamicBody(RN::PhysXCompoundShape::WithModel(model, physicsMaterial, false), 0.0f);
		_body->SetCollisionFilter(World::CollisionType::Level, World::CollisionType::All);
		_body->SetEnableKinematic(true);
		_door->AddAttachment(_body);
	}
	
	Door::~Door()
	{
//		SafeRelease(_controller);
	}
	
	void Door::Update(float delta)
	{
		RN::SceneNode::Update(delta);
		
		float distanceToPlayer = (World::GetSharedInstance()->GetPlayer()->GetWorldPosition()-GetWorldPosition()).GetLength();
		if(distanceToPlayer > 3.0f || !_isActive)
		{
			RN::Vector3 closeDirection = -_door->GetPosition();
			if(closeDirection.GetLength() > delta)
			{
				closeDirection.Normalize(delta);
			}
			_body->SetKinematicTarget(GetWorldRotation().GetRotatedVector(closeDirection)+_door->GetWorldPosition(), GetWorldRotation());
		}
		
		if(_isActive && distanceToPlayer <= 3.0f)
		{
			RN::Vector3 openDirection = _openOffset-_door->GetPosition();
			if(openDirection.GetLength() > delta)
			{
				openDirection.Normalize(delta);
			}
			_body->SetKinematicTarget(GetWorldRotation().GetRotatedVector(openDirection)+_door->GetWorldPosition(), GetWorldRotation());
		}
	}
	
	void Door::SetActive(bool active)
	{
		_isActive = active;
	}
}
