//
//  PZMeleeZombie.h
//  Project: Z
//
//  Copyright 2018 by Überpixel. All rights reserved.
//  Unauthorized use is punishable by torture, mutilation, and vivisection.
//

#ifndef __PROJECT_Z_MELEEZOMBIE_H_
#define __PROJECT_Z_MELEEZOMBIE_H_

#include <Rayne.h>
#include <RNRecastAgent.h>
#include "PZZombie.h"

namespace PZ
{
	class MeleeZombie : public Zombie
	{
	public:
		MeleeZombie();
		~MeleeZombie();

		void Update(float delta) override;
		
	private:
		RN::SteamAudioSource *_mouthSource;
		RN::Array *_attackSounds;
		RN::Array *_idleSounds;
		
		float _idleTimer;
	};
}

#endif /* defined(__PROJECT_Z_MELEEZOMBIE_H_) */
