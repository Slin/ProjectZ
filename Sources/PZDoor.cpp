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
	Door::Door(RN::String *filename, RN::Vector3 openOffset) : _state(State::Automatic), _openOffset(openOffset), _didSetInitialState(false), _needsReset(false), _isMoving(false)
	{
		RN::Model *model = RN::Model::WithName(filename);
		_door = new RN::Entity(model);
		AddChild(_door->Autorelease());
		
		RN::PhysXMaterial *physicsMaterial = new RN::PhysXMaterial();
		_body = new RN::PhysXDynamicBody(RN::PhysXCompoundShape::WithModel(model, physicsMaterial, false), 1.0f);
		_body->SetCollisionFilter(World::CollisionType::Doors, World::CollisionType::All);
		_body->SetEnableKinematic(true);
		_door->AddAttachment(_body);
		
		RN::AudioAsset *audioAsset = RN::AudioAsset::WithName(RNCSTR("audio/door2.ogg"));
		_doorSource = new RN::SteamAudioSource(audioAsset, false);
		_doorSource->SetTimeOfFlight(false);
		_doorSource->SetRadius(0.0f);
		AddChild(_doorSource->Autorelease());
		_doorSource->SetPosition(RN::Vector3(0.0f, 0.0f, 0.0f));
	}
	
	Door::~Door()
	{
//		SafeRelease(_controller);
	}
	
	void Door::Update(float delta)
	{
		if(!_didSetInitialState)
		{
			_initialState = _state;
			_didSetInitialState = true;
		}
		
		RN::SceneNode::Update(delta);
		
		bool playerIsDead = World::GetSharedInstance()->GetPlayer()->IsDead();
		if(playerIsDead)
		{
			_needsReset = true;
		}
		else if(_needsReset)
		{
			_state = _initialState;
			_needsReset = false;
		}
		
		bool isTriggered = false;
		
		RN::Vector3 from = GetWorldPosition();
		RN::Vector3 to = World::GetSharedInstance()->GetPlayer()->GetWorldPosition() + RN::Vector3(0.0f, 1.0f, 0.0f);
		RN::Vector3 diff = to - from;
		float distanceToPlayer = diff.GetLength();
		if(distanceToPlayer < 2.5f)
		{
			RN::PhysXContactInfo hit;
			hit.distance = -1.0f;
			if(distanceToPlayer > 0.1f)
			{
				hit = World::GetSharedInstance()->GetPhysicsWorld()->CastRay(from, to, World::CollisionType::Level);
			}
			
			if(hit.distance < -0.5f)
				isTriggered = true;
		}
		
		if((!isTriggered && _state == State::Automatic) || _state == State::Closed)
		{
			RN::Vector3 closeDirection = -_door->GetPosition();
			if(closeDirection.GetLength() > delta)
			{
				if(!_isMoving)
				{
					_doorSource->Seek(0.0f);
					_doorSource->Play();
				}
				
				closeDirection.Normalize(delta);
				_isMoving = true;
			}
			else
			{
				_doorSource->Stop();
				_isMoving = false;
			}
			_body->SetKinematicTarget(GetWorldRotation().GetRotatedVector(closeDirection)+_door->GetWorldPosition(), GetWorldRotation());
		}
		
		if((isTriggered && _state == State::Automatic) || _state == State::Opened)
		{
			RN::Vector3 openDirection = _openOffset-_door->GetPosition();
			if(openDirection.GetLength() > delta)
			{
				if(!_isMoving)
				{
					_doorSource->Seek(0.0f);
					_doorSource->Play();
				}
				
				openDirection.Normalize(delta);
				_isMoving = true;
			}
			else
			{
				_doorSource->Stop();
				_isMoving = false;
			}
			_body->SetKinematicTarget(GetWorldRotation().GetRotatedVector(openDirection)+_door->GetWorldPosition(), GetWorldRotation());
		}
	}
	
	void Door::SetState(State state)
	{
		_state = state;
	}
}
