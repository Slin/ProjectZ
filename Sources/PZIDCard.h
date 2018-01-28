//
//  PZIDCard.h
//  Project: Z
//
//  Copyright 2018 by Überpixel. All rights reserved.
//  Unauthorized use is punishable by torture, mutilation, and vivisection.
//

#ifndef __PROJECT_Z_IDCARD_H_
#define __PROJECT_Z_IDCARD_H_

#include <Rayne.h>
#include "RNPhysXWorld.h"

namespace PZ
{
	class IDCard : public RN::SceneNode
	{
	public:
		IDCard();
		~IDCard();
		
		void Update(float delta) override;

	private:
		RN::Entity *_entity;
		bool _taken;
	};
}

#endif /* defined(__PROJECT_Z_SWITCH_H_) */
