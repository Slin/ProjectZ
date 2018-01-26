//
//  PZWorld.h
//  Project: Z
//
//  Copyright 2018 by Überpixel. All rights reserved.
//  Unauthorized use is punishable by torture, mutilation, and vivisection.
//

#ifndef __PROJECT_Z_WORLD_H_
#define __PROJECT_Z_WORLD_H_

#include <Rayne.h>

#include "RNVRCamera.h"
#include "RNPhysXWorld.h"
#include "RNSteamAudioWorld.h"

#include "PZPlayer.h"

namespace PZ
{
	class World : public RN::Scene
	{
	public:
		static World *GetSharedInstance();
		static void Exit();

		World(RN::VRWindow *vrWindow, RN::Window *window, bool hasShadows, RN::uint8 msaa, bool debug);

		RN::PhysXWorld *GetPhysicsWorld() const { return _physicsWorld; }
		RN::SteamAudioWorld *GetAudioWorld() const { return _audioWorld; }
		RN::ShaderLibrary *GetShaderLibrary() const { return _shaderLibrary; }
		RN::SceneNode *GetCamera() const { return _mainCamera; }

	protected:
		void WillBecomeActive() override;

		void InitializePlatform();
		void CreateTestLevel();

		void UpdateForWindowSize() const;
		void WillUpdate(float delta) override;

		RN::ShaderLibrary *_shaderLibrary;

		RN::Camera *_shadowCamera;
		RN::VRCamera *_vrCamera;
		RN::SceneNode *_mainCamera;
		RN::Window *_window;
		RN::VRWindow *_vrWindow;
		bool _hasShadows;
		RN::uint8 _msaa;
		bool _debug;
		RN::Material *_copyEyeToScreenMaterial;

		RN::PhysXWorld *_physicsWorld;
		RN::SteamAudioWorld *_audioWorld;
		
		bool _isPaused;

		static World *_sharedInstance;
	};
}


#endif /* __PROJECT_Z_WORLD_H_ */
