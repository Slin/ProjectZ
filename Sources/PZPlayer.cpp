//
//  SGPlayer.cpp
//  Sword Game
//
//  Copyright 2017 by Überpixel. All rights reserved.
//  Unauthorized use is punishable by torture, mutilation, and vivisection.
//

#include "SGPlayer.h"
#include "SGWorld.h"

namespace SG
{
	Player::Player(RN::uint16 id, RN::Vector3 position) : NetworkedSceneNode(id), _health(1.0f)
	{
		SetWorldPosition(position);

		//Create the body entity
		RN::Mesh *boxMesh = RN::Mesh::WithColoredCube(RN::Vector3(0.28f, 0.5f, 0.08f), RN::Color::White());
		RN::Material *boxMaterial = RN::Material::WithShaders(RN::Renderer::GetActiveRenderer()->GetDefaultShader(RN::Shader::Type::Vertex, RN::Shader::Options::WithMesh(boxMesh)), RN::Renderer::GetActiveRenderer()->GetDefaultShader(RN::Shader::Type::Fragment, RN::Shader::Options::WithMesh(boxMesh)));
		boxMaterial->SetDiffuseColor(RN::Color::WithRGBA(0.0f, 1.0f, 0.0f));
		RN::Model *boxModel = new RN::Model(boxMesh, boxMaterial);
		_bodyEntity = new RN::Entity(boxModel);
		AddChild(_bodyEntity->Autorelease());

		//Create hands
		RN::Mesh *handGhostMesh = RN::Mesh::WithColoredCube(RN::Vector3(0.05f, 0.05f, 0.05f), RN::Color::White());
		RN::Material *handGhostMaterial = RN::Material::WithMaterial(boxMaterial);
		handGhostMaterial->SetDiffuseColor(RN::Color::WithRGBA(0.5f, 0.5f, 0.5f, 0.12f));
		handGhostMaterial->SetAlphaToCoverage(true, 0.0f, 1.0f);
		RN::Model *handGhostModel = new RN::Model(handGhostMesh, handGhostMaterial);

		RN::Mesh *handMesh = RN::Mesh::WithColoredCube(RN::Vector3(0.05f, 0.05f, 0.05f), RN::Color::White());
		RN::Material *handMaterial = RN::Material::WithMaterial(boxMaterial);
		handMaterial->SetDiffuseColor(RN::Color::WithRGBA(1.0f, 0.878f, 0.768f));
		RN::Model *handModel = new RN::Model(handMesh, handMaterial);

		for(int i = 0; i < 2; i++)
		{
			_hand[i] = new Hand(handGhostModel, handModel, handModel);
			AddChild(_hand[i]->Autorelease());
		}

		//Create the head entity
		RN::Model *headModel = RN::Model::WithName(RNCSTR("models/characters/face.sgm"));
		_head = new BodyPart(this, headModel, headModel);
		AddChild(_head->Autorelease());
	}
	
	Player::~Player()
	{
		//TODO: Cleanup
	}
	
	void Player::Update(float delta)
	{
		RN::SceneNode::Update(delta);
	}

	void Player::UpdateBody(RN::Vector3 direction)
	{
		//Calculate neck position
		RN::Vector3 eyeVector(0.0f, 0.25f, -0.08f);
		RN::Vector3 headPosition = _head->GetPosition();
		RN::Vector3 neckPosition = headPosition - _head->GetRotation().GetRotatedVector(eyeVector);

		//Scale body from feet to head
		float heightFeetToNeck = neckPosition.y;
		_bodyEntity->SetScale(RN::Vector3(1.0f, heightFeetToNeck, 1.0f));

		//Place body entity centered between ground and neck
		RN::Vector3 bodyPosition = neckPosition;
		bodyPosition.y = 0.0f;
		bodyPosition.y += heightFeetToNeck*0.5f;
		_bodyEntity->SetPosition(bodyPosition);

		//Body rotation
		RN::Vector3 bodyRotation = _bodyEntity->GetEulerAngle();
		if(direction.GetLength() > 0.05f)
		{
			bodyRotation = _hand[0]->GetWorldRotation().GetEulerAngle();
			bodyRotation.y = 0.0f;
			bodyRotation.z = 0.0f;
		}

		RN::Vector3 bodyForward = RN::Quaternion::WithEulerAngle(bodyRotation).GetRotatedVector(RN::Vector3(0.0f, 0.0f, -1.0f));
		bodyForward.y = 0.0f;
		bodyForward.Normalize();
		RN::Vector3 headForward = _head->GetForward();
		headForward.y = 0.0f;
		headForward.Normalize();
		if(bodyForward.GetDotProduct(headForward) < std::cos(70.0f / 180.0f*M_PI))
		{
			RN::Vector3 headRotation = _head->GetEulerAngle();
			float diff = headRotation.x - bodyRotation.x;
			if (diff > 180.0f) diff = headRotation.x - (360 + bodyRotation.x);
			if (diff < -180.0f) diff = 360 + headRotation.x - bodyRotation.x;
			if (diff > 0.0f)
			{
				headRotation.x -= 70.0f;
			}
			else
			{
				headRotation.x += 70.0f;
			}

			bodyRotation = headRotation;
			bodyRotation.y = 0.0f;
			bodyRotation.z = 0.0f;
		}

		_bodyEntity->SetRotation(bodyRotation);

		RN::Color healthColor = RN::Color::WithRGBA(1.0f - _health, _health, 0.0f);
		_bodyEntity->GetModel()->GetLODStage(0)->GetMaterialAtIndex(0)->SetDiffuseColor(healthColor);
	}

	void Player::DoDamage(float damage)
	{
		_health -= damage;
		
		if(_health < 0.0f)
			_health = 1.0f;
	}

	SGNetworking::Packet Player::ProcessPacket(const SGNetworking::Packet &packet)
	{
		SGNetworking::Packet updatedPacket = packet;
		if(World::GetSharedInstance()->GetIsServer())
		{
			updatedPacket.mutable_playerstate()->set_health(_health);
		}
		else
		{
			_health = packet.playerstate().health();
		}

		RN::uint32 leftHandHeldObjectID = packet.playerstate().lefthand().heldobjectid();
		if(leftHandHeldObjectID > 0)
		{
			_hand[0]->Grab(leftHandHeldObjectID);
		}
		else
		{
			_hand[0]->Drop();
		}

		RN::uint32 rightHandHeldObjectID = packet.playerstate().righthand().heldobjectid();
		if(rightHandHeldObjectID > 0)
		{
			_hand[1]->Grab(rightHandHeldObjectID);
		}
		else
		{
			_hand[1]->Drop();
		}

		return updatedPacket;
	}

	SGNetworking::Packet Player::CreatePacket() const
	{
		SGNetworking::Packet packet;

		if(World::GetSharedInstance()->GetIsServer())
		{
			packet.mutable_playerstate()->set_health(_health);
		}
		else
		{
			packet.mutable_playerstate()->set_health(-1.0f);
		}

		SGNetworking::PlayerState *playerState = packet.mutable_playerstate();

		SGNetworking::Head *head = playerState->mutable_head();
		RN::Vector3 headPosition = _head->GetWorldPosition();
		head->mutable_transform()->mutable_position()->set_x(headPosition.x);
		head->mutable_transform()->mutable_position()->set_y(headPosition.y);
		head->mutable_transform()->mutable_position()->set_z(headPosition.z);

		RN::Quaternion headRotation = _head->GetWorldRotation();
		head->mutable_transform()->mutable_rotation()->set_x(headRotation.x);
		head->mutable_transform()->mutable_rotation()->set_y(headRotation.y);
		head->mutable_transform()->mutable_rotation()->set_z(headRotation.z);
		head->mutable_transform()->mutable_rotation()->set_w(headRotation.w);

		SGNetworking::Hand *leftHand = playerState->mutable_lefthand();
		RN::Vector3 leftHandPosition = _hand[0]->GetWorldPosition();
		leftHand->mutable_transform()->mutable_position()->set_x(leftHandPosition.x);
		leftHand->mutable_transform()->mutable_position()->set_y(leftHandPosition.y);
		leftHand->mutable_transform()->mutable_position()->set_z(leftHandPosition.z);

		RN::Quaternion leftHandRotation = _hand[0]->GetWorldRotation();
		leftHand->mutable_transform()->mutable_rotation()->set_x(leftHandRotation.x);
		leftHand->mutable_transform()->mutable_rotation()->set_y(leftHandRotation.y);
		leftHand->mutable_transform()->mutable_rotation()->set_z(leftHandRotation.z);
		leftHand->mutable_transform()->mutable_rotation()->set_w(leftHandRotation.w);

		if(_hand[0]->GetHeldObject())
		{
			RN::uint32 heldObjectID = _hand[0]->GetHeldObject()->GetID();
			leftHand->set_heldobjectid(heldObjectID);
		}
		else
		{
			leftHand->set_heldobjectid(0);
		}

		SGNetworking::Hand *rightHand = playerState->mutable_righthand();
		RN::Vector3 rightHandPosition = _hand[1]->GetWorldPosition();
		rightHand->mutable_transform()->mutable_position()->set_x(rightHandPosition.x);
		rightHand->mutable_transform()->mutable_position()->set_y(rightHandPosition.y);
		rightHand->mutable_transform()->mutable_position()->set_z(rightHandPosition.z);

		RN::Quaternion rightHandRotation = _hand[1]->GetWorldRotation();
		rightHand->mutable_transform()->mutable_rotation()->set_x(rightHandRotation.x);
		rightHand->mutable_transform()->mutable_rotation()->set_y(rightHandRotation.y);
		rightHand->mutable_transform()->mutable_rotation()->set_z(rightHandRotation.z);
		rightHand->mutable_transform()->mutable_rotation()->set_w(rightHandRotation.w);

		if(_hand[1]->GetHeldObject())
		{
			RN::uint32 heldObjectID = _hand[1]->GetHeldObject()->GetID();
			rightHand->set_heldobjectid(heldObjectID);
		}
		else
		{
			rightHand->set_heldobjectid(0);
		}

		packet.mutable_playerstate()->set_objectid(_id);

		return packet;
	}
}
