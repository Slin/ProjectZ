//
//  PZStickSwitch.h
//  Project: Z
//
//  Copyright 2018 by Überpixel. All rights reserved.
//  Unauthorized use is punishable by torture, mutilation, and vivisection.
//

#ifndef __PROJECT_Z_STICKSWITCH_H_
#define __PROJECT_Z_STICKSWITCH_H_

#include "PZSwitch.h"

namespace PZ
{
	class StickSwitch : public Switch
	{
	public:
		StickSwitch();
		~StickSwitch();
		
		void Update(float delta) override;
		void SetActive(bool active) override;

	private:
	};
}

#endif /* defined(__PROJECT_Z_STICKSWITCH_H_) */