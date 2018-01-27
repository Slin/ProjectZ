//
//  PZPlayer.h
//  Project: Z
//
//  Copyright 2018 by Überpixel. All rights reserved.
//  Unauthorized use is punishable by torture, mutilation, and vivisection.
//

#ifndef __PROJECT_Z_PLAYER_H_
#define __PROJECT_Z_PLAYER_H_

#include <Rayne.h>
#include <RNPhysXWorld.h>

namespace PZ
{
	class World;
	class Player : public RN::SceneNode
	{
	public:
		Player(RN::SceneNode *camera);
		~Player();
		
		void Update(float delta) override;

		void Die();
		bool IsDead();
		
		bool IsActivatePressed();

	private:
		RN::SceneNode *_camera;
		RN::PhysXKinematicController *_controller;
		RN::InputDevice *_gamepad;

		RN::Vector3 _spawnPoint;
		RN::Quaternion _spawnRotation;
		bool _storeSpawnPoint;
		bool _dead;
		float _deathTime;
		float _invulnerableTime;
		float _cameraShakeTime;
		RN::Quaternion _cameraRotation;
	};
}

#endif /* defined(__PROJECT_Z_PLAYER_H_) */
