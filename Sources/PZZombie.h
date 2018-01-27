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

#define PZ_MAX_SPIT_BLOBS 64

namespace PZ
{
	typedef struct {
		RN::Entity *entity;
		RN::Vector3 velocity;
		bool active;
	} SpitBlob;

	class Zombie : public RN::SceneNode
	{
	public:
		Zombie();
		~Zombie();

		void Update(float delta) override;

	protected:
		bool ShouldSpit();

	private:
		RN::Entity *_zombie;
		RN::RecastAgent *_navigationAgent;
		RN::Vector3 _previousPosition;
		//		RN::PhysXKinematicController *_controller;

		SpitBlob _spitBlobs[PZ_MAX_SPIT_BLOBS];
		int _spitBlobIndex;
		float _spitNextTime;

		void RunSpit(float delta);
	};
}

#endif /* defined(__PROJECT_Z_ZOMBIE_H_) */
