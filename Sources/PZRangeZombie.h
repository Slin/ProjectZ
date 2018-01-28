//
//  PZRangeZombie.h
//  Project: Z
//
//  Copyright 2018 by Überpixel. All rights reserved.
//  Unauthorized use is punishable by torture, mutilation, and vivisection.
//

#ifndef __PROJECT_Z_RANGEZOMBIE_H_
#define __PROJECT_Z_RANGEZOMBIE_H_

#include <Rayne.h>
#include <RNRecastAgent.h>
#include "PZZombie.h"

#define PZ_MAX_SPIT_BLOBS 64

namespace PZ
{
	typedef struct {
		RN::Entity *entity;
		RN::Vector3 velocity;
		bool active;
	} SpitBlob;

	class RangeZombie : public Zombie
	{
	public:
		RangeZombie();
		~RangeZombie();

		void Update(float delta) override;

	private:
		SpitBlob _spitBlobs[PZ_MAX_SPIT_BLOBS];
		int _spitBlobIndex;
		float _spitNextTime;
		RN::SteamAudioSource *_mouthSource;
		RN::Array *_pukeSounds;

		void RunSpit(float delta);
	};
}

#endif /* defined(__PROJECT_Z_RANGEZOMBIE_H_) */
