//
//  PZDoor.h
//  Project: Z
//
//  Copyright 2018 by Überpixel. All rights reserved.
//  Unauthorized use is punishable by torture, mutilation, and vivisection.
//

#ifndef __PROJECT_Z_DOOR_H_
#define __PROJECT_Z_DOOR_H_

#include <Rayne.h>
#include "RNPhysXWorld.h"

namespace PZ
{
	class Door : public RN::SceneNode
	{
	public:
		Door(RN::String *filename, RN::Vector3 openOffset);
		~Door();
		
		void Update(float delta) override;
		
		void SetActive(bool active);

	private:
		RN::Entity *_door;
		RN::PhysXDynamicBody *_body;
		
		RN::Vector3 _openOffset;
		bool _isActive;
	};
}

#endif /* defined(__PROJECT_Z_DOOR_H_) */
