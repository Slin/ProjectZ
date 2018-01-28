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
#include <RNSteamAudioWorld.h>

namespace PZ
{
	class Zombie : public RN::SceneNode
	{
	public:
		Zombie(RN::Model *model, float normalSpeed = 2, float followSpeed = 3);
		~Zombie();

		void Update(float delta) override;

	private:
		RN::Entity *_zombie;
		RN::RecastAgent *_navigationAgent;
		RN::Vector3 _previousPosition;
		RN::SteamAudioSource *_slurpSource;

		float _normalSpeed;
		float _followSpeed;

		bool _following;
		float _followTime;
		float _slurpTime;
		bool _returnToSpawn;
		RN::Vector3 _spawnPoint;
		bool _storeSpawnPoint;
	};
}

#endif /* defined(__PROJECT_Z_ZOMBIE_H_) */
