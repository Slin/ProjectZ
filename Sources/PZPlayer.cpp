//
//  PZPlayer.cpp
//  Project: Z
//
//  Copyright 2018 by Überpixel. All rights reserved.
//  Unauthorized use is punishable by torture, mutilation, and vivisection.
//

#include "PZPlayer.h"
#include "PZWorld.h"

namespace PZ
{
	Player::Player(RN::SceneNode *camera) : _camera(camera->Retain()), _gamepad(nullptr)
	{
		AddChild(camera);
		camera->SetPosition(RN::Vector3(0.0f, 1.8f, 0.0f));
		
		RN::PhysXMaterial *physicsMaterial = new RN::PhysXMaterial();
		_controller = new RN::PhysXKinematicController(0.25f, 1.1f, physicsMaterial->Autorelease());
		_controller->SetPositionOffset(_controller->GetFeetOffset());
		_controller->SetCollisionFilter(World::CollisionType::Character, World::CollisionType::Level | World::CollisionType::Character);
		AddAttachment(_controller);
		
		RN::Array *gamepads = RN::InputManager::GetSharedInstance()->GetDevicesWithCategories(RN::InputDevice::Category::Gamepad);
		if(gamepads->GetCount() > 0)
		{
			_gamepad = gamepads->GetFirstObject<RN::InputDevice>();
			_gamepad->Activate();
		}

		_dead = false;
		_storeSpawnPoint = true;
		_invulnerableTime = 3;
		_deathTime = 0.0f;
		_invulnerableTime = 0.0f;
		_cameraShakeTime = 0.0f;
	}
	
	Player::~Player()
	{
		SafeRelease(_camera);
		SafeRelease(_controller);
	}
	
	void Player::Update(float delta)
	{
		if (_storeSpawnPoint) {
			_spawnPoint = GetPosition();
			_spawnRotation = GetRotation();
			_storeSpawnPoint = false;
		}

		if (_invulnerableTime > 0) {
			_invulnerableTime -= delta;
		}

		RN::SceneNode::Update(delta);
		
		RN::InputManager *manager = RN::InputManager::GetSharedInstance();

		RN::Vector3 rotation(0.0);
		rotation.x = manager->GetMouseDelta().x;
		rotation.y = manager->GetMouseDelta().y;
		rotation = -rotation;

		RN::Vector3 translation(0.0);
		if (!_dead) {
			translation.x = ((int)manager->IsControlToggling(RNCSTR("D")) - (int)manager->IsControlToggling(RNCSTR("A"))) * 3.0f;
			translation.z = ((int)manager->IsControlToggling(RNCSTR("S")) - (int)manager->IsControlToggling(RNCSTR("W"))) * 3.0f;
		}

		if (_gamepad)
		{
			RN::Object *leftAnalog = _gamepad->GetControlValue(RNCSTR("Analog Left"));

			if (leftAnalog)
			{
				RN::Vector2 left = leftAnalog->Downcast<RN::Value>()->GetValue<RN::Vector2>();
				if (!_dead) {
					translation.x += left.x * 3.0f;
					translation.z += left.y * 3.0f;
				}
			}

			RN::Object *rightAnalog = _gamepad->GetControlValue(RNCSTR("Analog Right"));

			if (rightAnalog)
			{
				RN::Vector2 right = rightAnalog->Downcast<RN::Value>()->GetValue<RN::Vector2>();
				rotation.x -= right.x * 20.0f;
				rotation.y = right.y * 20.0f;
			}
		}

		_camera->SetRotation(_cameraRotation);
		_camera->Rotate(rotation * delta * 15.0f);
		_cameraRotation = _camera->GetRotation();

		if (_cameraShakeTime > 0) {
			_cameraShakeTime -= delta;
			float a1 = (float(rand()) / float(RAND_MAX)) * 2.0f - 1.0f;
			float a2 = (float(rand()) / float(RAND_MAX)) * 2.0f - 1.0f;
			float a3 = (float(rand()) / float(RAND_MAX)) * 2.0f - 1.0f;
			_camera->Rotate(RN::Quaternion(RN::Vector3(a1 * 4.0f, a2 * 4.0f, a3 * 4.0f)));
		}

		RN::Vector3 globalTranslaion = RN::Quaternion(RN::Vector3(_camera->GetWorldEulerAngle().x, 0.0f, 0.0f)).GetRotatedVector(translation);
		_controller->Move(globalTranslaion*delta, delta);
		_controller->Gravity(-9.81f, delta);
		
		if (_dead) {
			_deathTime -= delta;
			if (_deathTime < 0) {
				_dead = false;
				SetPosition(_spawnPoint);
				SetRotation(_spawnRotation);
				_invulnerableTime = 3;
			}
		}
	}

	void Player::Die() {
		if (_dead || _invulnerableTime > 0) {
			return;
		}
		_dead = true;
		_deathTime = 3;
		_cameraShakeTime = 0.5f;
	}

	bool Player::IsDead() {
		return _dead;
	}
	
	bool Player::IsActivatePressed()
	{
		RN::InputManager *manager = RN::InputManager::GetSharedInstance();
		bool active = (int)manager->IsControlToggling(RNCSTR("E"));
		
		if(_gamepad && !active)
		{
			RN::ButtonControl *buttonCross = _gamepad->GetControlWithName<RN::ButtonControl>(RNCSTR("Button Cross"));
			if(buttonCross)
			{
				active = buttonCross->IsPressed();
			}
		}
		return active;
	}
}
