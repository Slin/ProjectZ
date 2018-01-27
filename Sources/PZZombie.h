//
//  PZZombie.h
//  Project: Z
//
//  Copyright 2018 by Überpixel. All rights reserved.
//  Unauthorized use is punishable by torture, mutilation, and vivisection.
//

#ifndef __PROJECT_Z_ZOMBIE_H_
#define __PROJECT_Z_ZOMBIE_H_

#include <Rayne.h>
#include <RNRecastAgent.h>

namespace PZ
{
	class Zombie : public RN::SceneNode
	{
	public:
		Zombie();
		~Zombie();
		
		void Update(float delta) override;

	private:
		RN::RecastAgent *_navigationAgent;
//		RN::PhysXKinematicController *_controller;
	};
}

#endif /* defined(__PROJECT_Z_ZOMBIE_H_) */
