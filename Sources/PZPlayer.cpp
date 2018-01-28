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
	Player::Player(RN::SceneNode *camera) : _camera(camera->Retain()), _gamepad(nullptr), _stepTimer(0.0f)
	{
		AddChild(camera);
		camera->SetPosition(RN::Vector3(0.0f, 1.8f, 0.0f));
		
		RN::PhysXMaterial *physicsMaterial = new RN::PhysXMaterial();
		_controller = new RN::PhysXKinematicController(0.25f, 1.1f, physicsMaterial->Autorelease());
		_controller->SetPositionOffset(_controller->GetFeetOffset());
		_controller->SetCollisionFilter(World::CollisionType::Character, World::CollisionType::All);
		AddAttachment(_controller);
		
		RN::Array *gamepads = RN::InputManager::GetSharedInstance()->GetDevicesWithCategories(RN::InputDevice::Category::Gamepad);
		if(gamepads->GetCount() > 0)
		{
			_gamepad = gamepads->GetFirstObject<RN::InputDevice>();
			_gamepad->Activate();
		}
		
		_stepSounds = new RN::Array();
		_stepSounds->AddObject(RN::AudioAsset::WithName(RNCSTR("audio/step1.ogg")));
		_stepSounds->AddObject(RN::AudioAsset::WithName(RNCSTR("audio/step2.ogg")));
		_stepSounds->AddObject(RN::AudioAsset::WithName(RNCSTR("audio/step3.ogg")));
		_stepSounds->AddObject(RN::AudioAsset::WithName(RNCSTR("audio/step4.ogg")));
		_stepSounds->AddObject(RN::AudioAsset::WithName(RNCSTR("audio/step5.ogg")));
		_stepSounds->AddObject(RN::AudioAsset::WithName(RNCSTR("audio/step6.ogg")));
		_stepSounds->AddObject(RN::AudioAsset::WithName(RNCSTR("audio/step7.ogg")));
		_stepSounds->AddObject(RN::AudioAsset::WithName(RNCSTR("audio/step8.ogg")));
		
		_dieSounds = new RN::Array();
		_dieSounds->AddObject(RN::AudioAsset::WithName(RNCSTR("audio/hit1.ogg")));
		_dieSounds->AddObject(RN::AudioAsset::WithName(RNCSTR("audio/hit2.ogg")));
		
		_leftStepSource = new RN::SteamAudioSource(nullptr, false);
		_leftStepSource->SetTimeOfFlight(false);
		_leftStepSource->SetRadius(0.0f);
		//_leftStepSource->SetGain(0.1f);
		AddChild(_leftStepSource->Autorelease());
		_leftStepSource->SetPosition(RN::Vector3(0.2f, 0.01f, -0.3f));
		
		_rightStepSource = new RN::SteamAudioSource(nullptr, false);
		_rightStepSource->SetTimeOfFlight(false);
		_rightStepSource->SetRadius(0.0f);
		//_leftStepSource->SetGain(0.1f);
		AddChild(_rightStepSource->Autorelease());
		_rightStepSource->SetPosition(RN::Vector3(-0.2f, 0.01f, -0.3f));
		
		_mouthSource = new RN::SteamAudioSource(nullptr, false);
		_mouthSource->SetTimeOfFlight(false);
		_mouthSource->SetRadius(0.0f);
		//_mouthSource->SetGain(0.1f);
		AddChild(_mouthSource->Autorelease());
		_mouthSource->SetPosition(RN::Vector3(0.0f, 1.65f, -0.1f));

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
		_stepSounds->Release();
		_dieSounds->Release();
	}
	
	void Player::Update(float delta)
	{
		RN::SceneNode::Update(delta);

		RN::InputManager *manager = RN::InputManager::GetSharedInstance();

		World *world = World::GetSharedInstance();

		static int startState = 0;
		if (startState == 0) {
			world->ShowUI(RNCSTR("start.png"));
			world->Fade(true, 1.0f);
			startState++;
			return;
		}
		else if (startState == 1) {
			if (world->IsFadeDone()) {
				startState++;
			}
			return;
		} else if (startState == 2) {
			if (manager->IsControlToggling(RNCSTR("W")) || IsActivatePressed()) {
				world->Fade(false, 1.0f);
				startState++;
			}
			return;
		}
		else if (startState == 3) {
			if (world->IsFadeDone()) {
				world->HideUI();
				world->Fade(true, 1.0f);
				startState++;
			}
			return;
		}
		else if (startState == 4) {
			if (world->IsFadeDone()) {
				startState++;
			}
			return;
		}

		if (_storeSpawnPoint) {
			_spawnPoint = GetPosition();
			_spawnRotation = GetRotation();
			_storeSpawnPoint = false;
		}

		if (_invulnerableTime > 0) {
			_invulnerableTime -= delta;
		}

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

		if (_cameraShakeTime > 0) {
			_camera->SetRotation(_cameraRotation);
		}
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
		
		if(translation.GetLength() > 0.1f)
		{
			_stepTimer -= delta;
			if((_leftStepSource->HasEnded() || !_leftStepSource->IsPlaying()) && (_rightStepSource->HasEnded() || !_rightStepSource->IsPlaying()) && _stepTimer <= 0.0f)
			{
				_stepTimer = 0.4f;
				int newStepIndex = RN::RandomNumberGenerator::GetSharedGenerator()->GetRandomInt32Range(0, _stepSounds->GetCount());
				float pitch = RN::RandomNumberGenerator::GetSharedGenerator()->GetRandomFloatRange(0.9f, 1.1f);
				if(_isLeftStep)
				{
					_leftStepSource->SetAudioAsset(_stepSounds->GetObjectAtIndex<RN::AudioAsset>(newStepIndex));
					_leftStepSource->SetPitch(pitch);
					_leftStepSource->Seek(0.0);
					_leftStepSource->Play();
					_isLeftStep = false;
				}
				else
				{
					_rightStepSource->SetAudioAsset(_stepSounds->GetObjectAtIndex<RN::AudioAsset>(newStepIndex));
					_rightStepSource->SetPitch(pitch);
					_leftStepSource->Seek(0.0);
					_rightStepSource->Play();
					_isLeftStep = true;
				}
				
				if(_gamepad)
					_gamepad->ExecuteCommand(RNCSTR("rumble"), RN::Number::WithUint8(50));
			}
			
			if(_gamepad && _stepTimer < 0.2f && _deathTime <= 0.0f)
				_gamepad->ExecuteCommand(RNCSTR("rumble"), RN::Number::WithUint8(0));
		}
		else
		{
			_stepTimer = 0.4f;
			if(_gamepad && _deathTime <= 0.0f)
				_gamepad->ExecuteCommand(RNCSTR("rumble"), RN::Number::WithUint8(0));
		}
		
		if (_dead) {
			_deathTime -= delta;
			
			if(_gamepad && _deathTime < 2.5f)
			{
				_gamepad->ExecuteCommand(RNCSTR("rumble"), RN::Number::WithUint8(0));
				_gamepad->ExecuteCommand(RNCSTR("light"), RN::Value::WithVector3(RN::Vector3(0.0f)));
			}
			
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
		
		int dieSoundIndex = RN::RandomNumberGenerator::GetSharedGenerator()->GetRandomInt32Range(0, _dieSounds->GetCount());
		_mouthSource->SetAudioAsset(_dieSounds->GetObjectAtIndex<RN::AudioAsset>(dieSoundIndex));
		_mouthSource->Seek(0.0);
		_mouthSource->Play();
		
		if(_gamepad)
		{
			_gamepad->ExecuteCommand(RNCSTR("light"), RN::Value::WithVector3(RN::Vector3(1.0f, 0.0f, 0.0f)));
			_gamepad->ExecuteCommand(RNCSTR("rumble"), RN::Number::WithUint8(255));
		}
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
