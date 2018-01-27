//
//  PZButtonSwitch.h
//  Project: Z
//
//  Copyright 2018 by Überpixel. All rights reserved.
//  Unauthorized use is punishable by torture, mutilation, and vivisection.
//

#ifndef __PROJECT_Z_BUTTONSWITCH_H_
#define __PROJECT_Z_BUTTONSWITCH_H_

#include "PZSwitch.h"

namespace PZ
{
	class ButtonSwitch : public Switch
	{
	public:
		ButtonSwitch();
		~ButtonSwitch();
		
		void Update(float delta) override;
		void SetActive(bool active) override;

	private:
		RN::Entity *_base;
		RN::Entity *_button;
		
	};
}

#endif /* defined(__PROJECT_Z_BUTTONSWITCH_H_) */
