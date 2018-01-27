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

namespace PZ
{
	class Switch : public RN::SceneNode
	{
	public:
		Switch();
		~Switch();
		
		void Update(float delta) override;
		virtual void SetActive(bool active);

	private:
		bool _isActive;
		bool _isKeyPressed;
	};
}

#endif /* defined(__PROJECT_Z_SWITCH_H_) */
