//
//  SGPlayer.h
//  Sword Game
//
//  Copyright 2017 by Überpixel. All rights reserved.
//  Unauthorized use is punishable by torture, mutilation, and vivisection.
//

#ifndef __SWORD_GAME_PLAYER_H_
#define __SWORD_GAME_PLAYER_H_

#include "SGNetworkedSceneNode.h"
#include "networking.pb.h"

#include "SGHand.h"
#include "SGBodyPart.h"

namespace SG
{
	class World;
	class Player : public NetworkedSceneNode
	{
	public:
		~Player() override;
		
		void Update(float delta) override;
		void UpdateBody(RN::Vector3 direction);

		SGNetworking::Packet ProcessPacket(const SGNetworking::Packet &packet) override;
		SGNetworking::Packet CreatePacket() const override;

		virtual void DoDamage(float damage);

	protected:
		Player(RN::uint16 id, RN::Vector3 position);

		float _health;

		BodyPart *_head;
		Hand *_hand[2];
		RN::Entity *_bodyEntity;
	};
}

#endif /* defined(__SWORD_GAME_PLAYER_H_) */
