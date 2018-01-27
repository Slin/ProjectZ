#pragma once
//
//  PZSpittingZombie.h
//  Project: Z
//
//  Copyright 2018 by Überpixel. All rights reserved.
//  Unauthorized use is punishable by torture, mutilation, and vivisection.
//

#ifndef __PROJECT_Z_SPITTINGZOMBIE_H_
#define __PROJECT_Z_SPITTINGZOMBIE_H_

#include <Rayne.h>
#include <RNRecastAgent.h>
#include "PZZombieBase.h"

#define PZ_MAX_SPIT_BLOBS 64

namespace PZ
{
	typedef struct {
		RN::Entity *entity;
		RN::Vector3 velocity;
		bool active;
	} SpitBlob;

	class SpittingZombie : public ZombieBase
	{
	public:
		SpittingZombie();
		~SpittingZombie();

		void Update(float delta) override;

	private:
		SpitBlob _spitBlobs[PZ_MAX_SPIT_BLOBS];
		int _spitBlobIndex;
		float _spitNextTime;

		void RunSpit(float delta);
	};
}

#endif /* defined(__PROJECT_Z_ZOMBIE_H_) */
