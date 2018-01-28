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
#include "RNRecastWorld.h"

#include "PZPlayer.h"
#include "PZRangeZombie.h"
#include "PZMeleeZombie.h"
#include "PZDoor.h"
#include "PZButtonSwitch.h"
#include "PZStickSwitch.h"
#include "PZIDCard.h"

namespace PZ
{
	class World : public RN::Scene
	{
	public:
		enum CollisionType
		{
			Level = 1 << 0,
			Character = 1 << 1,
			Doors = 1 << 2,
			
			All = 0xffffffff
		};
		
		static World *GetSharedInstance();
		static void Exit();

		World(RN::VRWindow *vrWindow, RN::Window *window, bool hasShadows, RN::uint8 msaa, bool debug);

		RN::PhysXWorld *GetPhysicsWorld() const { return _physicsWorld; }
		RN::SteamAudioWorld *GetAudioWorld() const { return _audioWorld; }
		RN::ShaderLibrary *GetShaderLibrary() const { return _shaderLibrary; }
		RN::SceneNode *GetCamera() const { return _mainCamera; }
		Player *GetPlayer() const { return _player; }

		bool IsPlayerVisibleFrom(RN::Vector3);

		void ShowUI(const RN::String *file);
		void HideUI();
		bool IsInUI();
		void Fade(bool in, float seconds);
		bool IsFadeDone();

	protected:
		void WillBecomeActive() override;

		void InitializePlatform();
		void CreateTestLevel();

		void UpdateForWindowSize() const;
		void WillUpdate(float delta) override;

		RN::ShaderLibrary *_shaderLibrary;

		Player *_player;
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
		RN::RecastWorld *_navigationWorld;

		RN::Entity *_uiEntity;
		RN::Vector3 _uiPlayerPositionBackup;
		RN::Quaternion _uiPlayerRotationBackup;
		RN::Vector3 _currentFadeColor;
		
		bool _isPaused;

		static World *_sharedInstance;

		float _fadeTime;
		float _fadeTimeTotal;
		bool _fadingIn;
		bool _fadingOut;
		bool _fadeDone;
	};
}


#endif /* __PROJECT_Z_WORLD_H_ */
