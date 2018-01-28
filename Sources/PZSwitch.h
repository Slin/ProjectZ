//
//  PZSwitch.h
//  Project: Z
//
//  Copyright 2018 by Überpixel. All rights reserved.
//  Unauthorized use is punishable by torture, mutilation, and vivisection.
//

#ifndef __PROJECT_Z_SWITCH_H_
#define __PROJECT_Z_SWITCH_H_

#include <Rayne.h>
#include "RNPhysXWorld.h"
#include "RNSteamAudioWorld.h"

namespace PZ
{
	class Switch : public RN::SceneNode
	{
	public:
		Switch();
		~Switch();
		
		void Update(float delta) override;
		virtual void SetActive(bool active);
		
		void SetAction(std::function< void(bool) > action);
		void SetNeedsKey(bool needsKey) {_needsKey = needsKey;}
		
	protected:
		bool _isActive;
		RN::Vector3 _raycastOffset;

	private:
		std::function< void(bool) > _action;
		bool _isKeyPressed;
		bool _needsReset;
		bool _needsKey;
		RN::SteamAudioSource *_lockedSource;
	};
}

#endif /* defined(__PROJECT_Z_SWITCH_H_) */
