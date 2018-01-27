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

	private:
		RN::SceneNode *_camera;
		RN::PhysXKinematicController *_controller;
		RN::InputDevice *_gamepad;
	};
}

#endif /* defined(__PROJECT_Z_PLAYER_H_) */
