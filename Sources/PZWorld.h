//
//  SGWorld.h
//  Sword Game
//
//  Copyright 2017 by Überpixel. All rights reserved.
//  Unauthorized use is punishable by torture, mutilation, and vivisection.
//

#ifndef __SWORD_GAME_WORLD_H_
#define __SWORD_GAME_WORLD_H_

#include <Rayne.h>

#include "RNVRCamera.h"
#include "RNNewtonWorld.h"
#include "RNSteamAudioWorld.h"
#include "RNENetWorld.h"

#include "networking.pb.h"

#include "SGServer.h"
#include "SGClient.h"
#include "SGPlayer.h"
#include "SGInterpolatedPlayer.h"
#include "SGInteractivePlayer.h"
#include "SGDummyAIPlayer.h"
#include "SGSingleGripObject.h"
#include "SGGrabbableObject.h"
#include "SGSpeech.h"
#include "SGPlatformWrapper.h"
#include "SGObjectManager.h"

namespace SG
{
	class World : public RN::Scene
	{
	public:
		enum CollisionType
		{
			Level = 1 << 0,
			Weapon = 1 << 1,
			Character = 1 << 2,
			CharacterPart = 1 << 3,

			All = 0xffffffff
		};
		
		static World *GetSharedInstance();
		static void Exit();

		World(bool isClient, bool isServer, RN::VRWindow *vrWindow, RN::Window *window, bool hasShadows, RN::uint8 msaa, bool debug);

		RN::NewtonWorld *GetPhysicsWorld() const { return _physicsWorld; }
		RN::SteamAudioWorld *GetAudioWorld() const { return _audioWorld; }
		RN::ENetHost *GetNetworkHost() const { return _networkHost; }
		RN::ShaderLibrary *GetShaderLibrary() const { return _shaderLibrary; }
		RN::VRCamera *GetVRCamera() const { return _vrCamera; }

		bool GetIsServer() const { return _isServer; }
		bool GetIsClient() const { return _isClient; }

	protected:
		void WillBecomeActive() override;

		void InitializePlatform();
		void InitializeNetworking();
		void CreateTestLevel();

		void UpdateForWindowSize() const;
		void WillUpdate(float delta) override;

		bool _isClient;
		bool _isServer;

		RN::ShaderLibrary *_shaderLibrary;
		RN::Entity *_ground;

		RN::Camera *_shadowCamera;
		RN::VRCamera *_vrCamera;
		RN::Window *_window;
		RN::VRWindow *_vrWindow;
		bool _hasShadows;
		RN::uint8 _msaa;
		bool _debug;
		RN::Material *_copyEyeToScreenMaterial;

		RN::NewtonWorld *_physicsWorld;
		RN::SteamAudioWorld *_audioWorld;
		RN::ENetWorld *_networkWorld;
		RN::ENetHost *_networkHost;
		bool _isPaused;
		
		bool _didSpawnSomething;

		Speech *_speech;

		RN::AudioAsset *_microphoneAudioAsset;

		static World *_sharedInstance;
	};
}


#endif /* __SWORD_GAME_WORLD_H_ */
