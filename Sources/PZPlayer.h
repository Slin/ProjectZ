//
//  PZPlayer.h
//  Projecgt: Z
//
//  Copyright 2018 by Überpixel. All rights reserved.
//  Unauthorized use is punishable by torture, mutilation, and vivisection.
//

#ifndef __PROJECT_Z_PLAYER_H_
#define __PROJECT_Z_PLAYER_H_

#include <Rayne.h>

namespace PZ
{
	class World;
	class Player : public RN::SceneNode
	{
	public:
		Player();
		~Player();
		
		void Update(float delta) override;

	protected:
		
	};
}

#endif /* defined(__PROJECT_Z_PLAYER_H_) */
