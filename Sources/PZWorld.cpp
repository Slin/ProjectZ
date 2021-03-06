//
//  PZWorld.cpp
//  Project: Z
//
//  Copyright 2018 by Überpixel. All rights reserved.
//  Unauthorized use is punishable by torture, mutilation, and vivisection.
//

#include "PZWorld.h"

#if RN_PLATFORM_WINDOWS
#include "RNOculusWindow.h"
#endif

namespace PZ
{
	World *World::_sharedInstance = nullptr;

	World *World::GetSharedInstance()
	{
		return _sharedInstance;
	}

	void World::Exit()
	{
		exit(0);
		//RN::Kernel::GetSharedInstance()->Exit();
	}

	World::World(RN::VRWindow *vrWindow, RN::Window *window, bool hasShadows, RN::uint8 msaa, bool debug) : _window(nullptr), _vrWindow(nullptr), _hasShadows(hasShadows), _msaa(msaa), _debug(debug), _audioWorld(nullptr), _vrCamera(nullptr), _isPaused(false), _shadowCamera(nullptr), _uiEntity(nullptr)
	{
		_fadingIn = false;
		_fadingOut = false;
		_fadeDone = false;

		if (vrWindow)
			_vrWindow = vrWindow->Retain();

		if (window)
			_window = window->Retain();
	}

	void World::WillBecomeActive()
	{
		RN::Scene::WillBecomeActive();
		_shaderLibrary = RN::Renderer::GetActiveRenderer()->CreateShaderLibraryWithFile(RNCSTR("shaders/Shaders.json"));
		_sharedInstance = this;

		//TODO: Renderer crasht bei schnell aus dem level bewegen
		
		InitializePlatform();

		_physicsWorld = new RN::PhysXWorld(RN::Vector3(0.0, -9.81, 0.0), true);
		AddAttachment(_physicsWorld);
		
		_navigationWorld = new RN::RecastWorld();
		AddAttachment(_navigationWorld);

		if(_vrCamera)
		{
			_vrCamera->SetClipFar(1500.0f);

			_shadowCamera = new RN::Camera();
			_shadowCamera->SetFlags(_shadowCamera->GetFlags() | RN::Camera::Flags::NoRender);
			_vrCamera->GetHead()->AddChild(_shadowCamera->Autorelease());
			AddNode(_vrCamera);
		}
		
		RN::SteamAudioPlayer *atmoAudioPlayer = _audioWorld->PlaySound(RN::AudioAsset::WithName(RNCSTR("audio/atmo.ogg")));
		atmoAudioPlayer->SetRepeat(true);
		
		_player = new Player(_mainCamera);
		AddNode(_player->Autorelease());
		CreateTestLevel();
		_player->SetWorldPosition(RN::Vector3(-14.0f, 0.0f, 0.0f));
		_mainCamera->SetWorldRotation(RN::Quaternion::WithEulerAngle(RN::Vector3(180, 0, 0)));
	}

	void World::InitializePlatform()
	{
		if(_vrWindow)
		{
			RN::PostProcessingStage *monitorPass = nullptr;
			if(_window)
			{
				monitorPass = new RN::PostProcessingStage();
				_copyEyeToScreenMaterial = RN::Material::WithShaders(_shaderLibrary->GetShaderWithName(RNCSTR("pp_vertex")), _shaderLibrary->GetShaderWithName(RNCSTR("pp_blit_fragment")));
				monitorPass->SetFramebuffer(_window->GetFramebuffer());
				monitorPass->SetMaterial(_copyEyeToScreenMaterial);
				monitorPass->Autorelease();
			}

			RN::Window *debugWindow = nullptr;
			if(_debug)
			{
				debugWindow = RN::Renderer::GetActiveRenderer()->CreateAWindow(_vrWindow->GetSize(), RN::Screen::GetMainScreen());
			}

			_vrCamera = new RN::VRCamera(_vrWindow, monitorPass, _msaa, debugWindow);
			_mainCamera = _vrCamera;
		}
		else if(RN::Renderer::GetActiveRenderer()->GetMainWindow())
		{
			RN::Window *mainWindow = RN::Renderer::GetActiveRenderer()->GetMainWindow();
			RN::Texture *msaaTexture = RN::Texture::WithDescriptor(RN::Texture::Descriptor::With2DRenderTargetFormatAndMSAA(RN::Texture::Format::BGRA8888SRGB, mainWindow->GetSize().x, mainWindow->GetSize().y, 8));
			RN::Texture *msaaDepthTexture = RN::Texture::WithDescriptor(RN::Texture::Descriptor::With2DRenderTargetFormatAndMSAA(RN::Texture::Format::Depth24Stencil8, mainWindow->GetSize().x, mainWindow->GetSize().y, 8));
			RN::Framebuffer *msaaFramebuffer = RN::Renderer::GetActiveRenderer()->CreateFramebuffer(mainWindow->GetSize());
			msaaFramebuffer->SetColorTarget(RN::Framebuffer::TargetView::WithTexture(msaaTexture));
			msaaFramebuffer->SetDepthStencilTarget(RN::Framebuffer::TargetView::WithTexture(msaaDepthTexture));
			RN::PostProcessingAPIStage *resolvePass = new RN::PostProcessingAPIStage(RN::PostProcessingAPIStage::Type::ResolveMSAA);

			RN::Texture *copyTexture = RN::Texture::WithDescriptor(RN::Texture::Descriptor::With2DRenderTargetFormat(RN::Texture::Format::BGRA8888SRGB, mainWindow->GetSize().x, mainWindow->GetSize().y));
			RN::Framebuffer *copyFramebuffer = RN::Renderer::GetActiveRenderer()->CreateFramebuffer(mainWindow->GetSize());
			copyFramebuffer->SetColorTarget(RN::Framebuffer::TargetView::WithTexture(copyTexture));
			resolvePass->SetFramebuffer(copyFramebuffer);

			RN::PostProcessingStage *monitorPass = new RN::PostProcessingStage();
			_copyEyeToScreenMaterial = RN::Material::WithShaders(_shaderLibrary->GetShaderWithName(RNCSTR("pp_vertex")), _shaderLibrary->GetShaderWithName(RNCSTR("pp_blit_fragment")));
			_copyEyeToScreenMaterial->SetAmbientColor(RN::Color(0.0f, 0.0f, 0.0f, 1.0f));
			monitorPass->SetFramebuffer(_window->GetFramebuffer());
			monitorPass->SetMaterial(_copyEyeToScreenMaterial);
			resolvePass->AddRenderPass(monitorPass);

			RN::Camera *camera = new RN::Camera();
			_mainCamera = camera;
			camera->GetRenderPass()->SetFramebuffer(msaaFramebuffer);
			camera->GetRenderPass()->AddRenderPass(resolvePass);
			camera->GetRenderPass()->SetClearColor(RN::Color::Black());
			camera->GetRenderPass()->SetFlags(RN::RenderPass::Flags::ClearColor|RN::RenderPass::Flags::ClearDepthStencil);
			
			_shadowCamera = camera;
		}

		RN::SteamAudioDevice *foundOutputAudioDevice = nullptr;

#if RN_PLATFORM_WINDOWS
		if(_vrWindow && _vrWindow->IsKindOfClass(RN::OculusWindow::GetMetaClass()))
		{
			RN::OculusWindow *oculusWindow = _vrWindow->Downcast<RN::OculusWindow>();
			const RN::String *oculusOutputAudioDeviceID = oculusWindow->GetPreferredAudioOutputDeviceID();
			const RN::String *oculusInputAudioDeviceID = oculusWindow->GetPreferredAudioInputDeviceID();
			RN::Array *audioDevice = RN::SteamAudioWorld::GetDevices();
			if(oculusOutputAudioDeviceID)
			{
				audioDevice->Enumerate<RN::SteamAudioDevice>([&](RN::SteamAudioDevice *audioDevice, size_t index, bool &stop){
					if(audioDevice->id->IsEqual(oculusOutputAudioDeviceID))
					{
						foundOutputAudioDevice = audioDevice;
						stop = true;
					}
				});
			}
		}
#endif

		if(!foundOutputAudioDevice)
		{
			foundOutputAudioDevice = RN::SteamAudioWorld::GetDefaultOutputDevice();
		}

		if(foundOutputAudioDevice)
		{
			_audioWorld = new RN::SteamAudioWorld(foundOutputAudioDevice, 3, 48000, 480);
			AddAttachment(_audioWorld);

			if(_vrCamera)
				_audioWorld->SetListener(_vrCamera->GetHead());
			else if(_shadowCamera)
				_audioWorld->SetListener(_shadowCamera);
		}
	}

	void World::CreateTestLevel()
	{
		if(_shadowCamera)
		{
			RN::Light *sunLight = new RN::Light(RN::Light::Type::DirectionalLight);
			sunLight->SetWorldRotation(RN::Vector3(45.0f + 90, -45.0f, 0.0f));
			AddNode(sunLight);
			
			if(_hasShadows)
				sunLight->ActivateShadows(RN::ShadowParameter(_shadowCamera));
		}

		RN::Model *levelModel = RN::Model::WithName(RNCSTR("models/levels/level1.sgm"));
		RN::Entity *level = new RN::Entity(levelModel);
		AddNode(level->Autorelease());

		//TODO: Mesh braucht eine optionale kopie im RAM, damit das trianglemeshshape korrekt funktioniert
		RN::PhysXMaterial *levelPhysicsMaterial = new RN::PhysXMaterial();
		RN::PhysXCompoundShape *levelShape = RN::PhysXCompoundShape::WithModel(levelModel, levelPhysicsMaterial, true);
		RN::PhysXStaticBody *levelBody = RN::PhysXStaticBody::WithShape(levelShape);
		levelBody->SetCollisionFilter(CollisionType::Level, CollisionType::All);
		level->AddAttachment(levelBody);
		
		_navigationWorld->SetRecastMesh(RN::RecastMesh::WithModel(levelModel), 10);

		if(_audioWorld)
		{
			RN::SteamAudioMaterial groundAudioMaterial;
			groundAudioMaterial.lowFrequencyAbsorption = 0.03f;
			groundAudioMaterial.midFrequencyAbsorption = 0.07f;
			groundAudioMaterial.highFrequencyAbsorption = 0.1f;
			groundAudioMaterial.scattering = 0.1f;
			_audioWorld->AddMaterial(groundAudioMaterial);

/*			RN::SteamAudioMaterial wallAudioMaterial;
			wallAudioMaterial.lowFrequencyAbsorption = 0.03f;
			wallAudioMaterial.midFrequencyAbsorption = 0.07f;
			wallAudioMaterial.highFrequencyAbsorption = 0.1f;
			wallAudioMaterial.scattering = 0.3f;
			_audioWorld->AddMaterial(wallAudioMaterial);*/

			RN::Model::LODStage *lodStage = levelModel->GetLODStage(0);
			for(int i = 0; i < lodStage->GetCount(); i++)
			{
				//RN::Material *material = lodStage->GetMaterialAtIndex(0);
				RN::SteamAudioGeometry groundAudioGeometry;
				groundAudioGeometry.mesh = lodStage->GetMeshAtIndex(i);
				groundAudioGeometry.materialIndex = 0;
				groundAudioGeometry.position = level->GetWorldPosition();
				groundAudioGeometry.scale = level->GetWorldScale();
				groundAudioGeometry.rotation = level->GetWorldRotation();
				_audioWorld->AddStaticGeometry(groundAudioGeometry);
			}

			_audioWorld->UpdateScene();
			
			RN::AudioAsset *audioAsset = RN::AudioAsset::WithName(RNCSTR("audio/elevatormusic.ogg"));
			RN::SteamAudioSource *musicSource = new RN::SteamAudioSource(audioAsset, false);
			musicSource->SetWorldPosition(RN::Vector3(26.9f, 1.0f, -3.5f));
			musicSource->SetTimeOfFlight(false);
			musicSource->Play();
			musicSource->SetRepeat(true);
			musicSource->SetRadius(0.0f);
			musicSource->SetGain(0.3f);
			AddNode(musicSource);
		}
		
		Switch *switch_;
		
		Door *door = new Door(RNCSTR("models/levels/door0.sgm"), RN::Vector3(0.0f, 0.0f, -1.8f));
		AddNode(door);
		door->SetState(Door::State::Automatic);
		door->SetWorldPosition(RN::Vector3(-12.0f, 1.25f, 4.0f));
		
		door = new Door(RNCSTR("models/levels/door1.sgm"), RN::Vector3(1.8f, 0.0f, 0.0f));
		AddNode(door);
		door->SetState(Door::State::Closed);
		door->SetWorldPosition(RN::Vector3(-6.0f, 1.25f, 3.0f));
		
		switch_ = new ButtonSwitch();
		AddNode(switch_);
		switch_->SetNeedsKey(false);
		switch_->SetWorldPosition(RN::Vector3(-4.5f, 1.0f, 3.1f));
		switch_->SetWorldRotation(RN::Quaternion::WithEulerAngle(RN::Vector3(180.0f, 0.0f, 0.0f)));
		switch_->SetAction([door](bool active){
			door->SetState(Door::State::Opened);
		});
		
		door = new Door(RNCSTR("models/levels/door2.sgm"), RN::Vector3(0.0f, 0.0f, -1.8f));
		AddNode(door);
		door->SetState(Door::State::Closed);
		door->SetWorldPosition(RN::Vector3(8.0f, 1.25f, -3.0f));
		
		switch_ = new ButtonSwitch();
		AddNode(switch_);
		switch_->SetWorldPosition(RN::Vector3(7.9f, 1.0f, -1.5f));
		switch_->SetWorldRotation(RN::Quaternion::WithEulerAngle(RN::Vector3(90.0f, 0.0f, 0.0f)));
		switch_->SetAction([door](bool active){
			door->SetState(Door::State::Automatic);
		});
		
		door = new Door(RNCSTR("models/levels/door3.sgm"), RN::Vector3(1.8f, 0.0f, 0.0f));
		AddNode(door);
		door->SetState(Door::State::Closed);
		door->SetWorldPosition(RN::Vector3(2.0f, 1.25f, 3.0f));
		
		switch_ = new ButtonSwitch();
		AddNode(switch_);
		switch_->SetWorldPosition(RN::Vector3(3.5f, 1.0f, 3.1f));
		switch_->SetWorldRotation(RN::Quaternion::WithEulerAngle(RN::Vector3(180.0f, 0.0f, 0.0f)));
		switch_->SetAction([door](bool active){
			door->SetState(Door::State::Automatic);
		});
		
		Door *elevatorDoor1 = new Door(RNCSTR("models/levels/elevator_left.sgm"), RN::Vector3(0.0f, 0.0f, -0.84f));
		elevatorDoor1->SetState(Door::State::Closed);
		AddNode(elevatorDoor1);
		elevatorDoor1->SetWorldPosition(RN::Vector3(24.0f, 1.25f, -4.0f));
		
		Door *elevatorDoor2 = new Door(RNCSTR("models/levels/elevator_right.sgm"), RN::Vector3(0.0f, 0.0f, 0.84f));
		elevatorDoor2->SetState(Door::State::Closed);
		AddNode(elevatorDoor2);
		elevatorDoor2->SetWorldPosition(RN::Vector3(24.0f, 1.25f, -3.0f));
		
		switch_ = new ButtonSwitch();
		AddNode(switch_);
		switch_->SetWorldPosition(RN::Vector3(23.9f, 1.0f, -4.75f));
		switch_->SetWorldRotation(RN::Quaternion::WithEulerAngle(RN::Vector3(90.0f, 0.0f, 0.0f)));
		switch_->SetAction([elevatorDoor1, elevatorDoor2](bool active){
			elevatorDoor1->SetState(Door::State::Opened);
			elevatorDoor2->SetState(Door::State::Opened);
		});
		
		
		door = new Door(RNCSTR("models/levels/extension.sgm"), RN::Vector3(0.0f, 0.0f, -1.8f));
		door->SetState(Door::State::Closed);
		AddNode(door);
		door->SetWorldPosition(RN::Vector3(8.0f, 1.25f, 16.0f));
		
		door = new Door(RNCSTR("models/levels/gate.sgm"), RN::Vector3(2.0f, 0.0f, 0.0f));
		door->SetState(Door::State::Closed);
		AddNode(door);
		door->SetWorldPosition(RN::Vector3(21.0f, 1.25f, -9.0f));
		
		
		switch_ = new StickSwitch();
		AddNode(switch_);
		switch_->SetWorldPosition(RN::Vector3(25.0f, 1.0f, -1.9f));
		switch_->SetWorldRotation(RN::Quaternion::WithEulerAngle(RN::Vector3(90.0f, 0.0f, 0.0f)));
		switch_->SetAction([door](bool active){
			door->SetState(door->GetState() == Door::State::Closed?Door::State::Opened:Door::State::Closed);
		});
		
		switch_ = new StickSwitch();
		AddNode(switch_);
		switch_->SetWorldPosition(RN::Vector3(24.1f, 1.0f, -10.0f));
		switch_->SetWorldRotation(RN::Quaternion::WithEulerAngle(RN::Vector3(180.0f, 0.0f, 0.0f)));
		switch_->SetAction([door](bool active){
			door->SetState(door->GetState() == Door::State::Closed?Door::State::Opened:Door::State::Closed);
		});
		
		door = new Door(RNCSTR("models/levels/door4.sgm"), RN::Vector3(-1.8f, 0.0f, 0.0f));
		AddNode(door);
		door->SetState(Door::State::Closed);
		door->SetWorldPosition(RN::Vector3(-6.0f, 1.25f, -7.0f));
		
		switch_ = new ButtonSwitch();
		AddNode(switch_);
		switch_->SetNeedsKey(false);
		switch_->SetWorldPosition(RN::Vector3(-4.5f, 1.0f, -6.9f));
		switch_->SetWorldRotation(RN::Quaternion::WithEulerAngle(RN::Vector3(180.0f, 0.0f, 0.0f)));
		switch_->SetAction([door](bool active){
			door->SetState(Door::State::Automatic);
		});
		
		door = new Door(RNCSTR("models/levels/door5.sgm"), RN::Vector3(-1.8f, 0.0f, 0.0f));
		AddNode(door);
		door->SetState(Door::State::Closed);
		door->SetWorldPosition(RN::Vector3(18.0f, 1.25f, -2.0f));
		
		switch_ = new ButtonSwitch();
		AddNode(switch_);
		switch_->SetWorldPosition(RN::Vector3(19.3f, 1.0f, -1.9f));
		switch_->SetWorldRotation(RN::Quaternion::WithEulerAngle(RN::Vector3(180.0f, 0.0f, 0.0f)));
		switch_->SetAction([door](bool active){
			door->SetState(Door::State::Automatic);
		});
		
		door = new Door(RNCSTR("models/levels/door6.sgm"), RN::Vector3(-1.8f, 0.0f, 0.0f));
		AddNode(door);
		door->SetState(Door::State::Closed);
		door->SetWorldPosition(RN::Vector3(14.0f, 1.25f, 3.0f));
		
		switch_ = new ButtonSwitch();
		AddNode(switch_);
		switch_->SetWorldPosition(RN::Vector3(15.3f, 1.0f, 3.1f));
		switch_->SetWorldRotation(RN::Quaternion::WithEulerAngle(RN::Vector3(180.0f, 0.0f, 0.0f)));
		switch_->SetAction([door](bool active){
			door->SetState(Door::State::Automatic);
		});
		
		
		Zombie *zombie = new MeleeZombie();
		AddNode(zombie->Autorelease());
		zombie->SetWorldPosition(RN::Vector3(-5.62, 0.0, 1.27));
		
		zombie = new RangeZombie();
		AddNode(zombie->Autorelease());
		zombie->SetWorldPosition(RN::Vector3(-6.0, 0.0, -12.0));
		
		zombie = new RangeZombie();
		AddNode(zombie->Autorelease());
		zombie->SetWorldPosition(RN::Vector3(6.0, 0.0, 12.0));
		
		zombie = new MeleeZombie();
		AddNode(zombie->Autorelease());
		zombie->SetWorldPosition(RN::Vector3(24.0, 0.0, 8.0));
		
		zombie = new MeleeZombie();
		AddNode(zombie->Autorelease());
		zombie->SetWorldPosition(RN::Vector3(18.0, 0.0, 0.5));

		IDCard *idCard = new IDCard();
		AddNode(idCard->Autorelease());
		idCard->SetWorldPosition(RN::Vector3(-6.4f, 1.065, -11.7f));
	}

	void World::UpdateForWindowSize() const
	{
		if(!_vrCamera || !_window)
		{
			if(_copyEyeToScreenMaterial)
			{
				_copyEyeToScreenMaterial->SetDiffuseColor(RN::Color(0.0f, 0.0f, 1.0f, 1.0f));
				_copyEyeToScreenMaterial->SetAmbientColor(RN::Color(_currentFadeColor.x, _currentFadeColor.y, _currentFadeColor.z, 1.0f));
			}
			return;
		}

		RN::Vector2 eyeResolution((_vrWindow->GetSize().x - _vrWindow->GetEyePadding()) / 2.0f, _vrWindow->GetSize().y);
		RN::Vector2 windowResolution(_window->GetSize());
		float eyeAspect = eyeResolution.x / eyeResolution.y;
		float windowAspect = windowResolution.x / windowResolution.y;
		RN::Vector4 sourceRectangle = 0.0f;

		if(eyeAspect < windowAspect)
		{
			sourceRectangle.z = eyeResolution.x;
			sourceRectangle.w = eyeResolution.x / windowAspect;
			sourceRectangle.x = 0.0f;
			sourceRectangle.y = (eyeResolution.y - sourceRectangle.w) * 0.5f;
		}
		else
		{
			sourceRectangle.z = eyeResolution.y * windowAspect;
			sourceRectangle.w = eyeResolution.y;
			sourceRectangle.x = (eyeResolution.x - sourceRectangle.z) * 0.5f;
			sourceRectangle.y = 0.0f;
		}

		_copyEyeToScreenMaterial->SetDiffuseColor(RN::Color(sourceRectangle.x / _vrWindow->GetSize().x, sourceRectangle.y / _vrWindow->GetSize().y, (sourceRectangle.z / _vrWindow->GetSize().x), (sourceRectangle.w / _vrWindow->GetSize().y)));
		_copyEyeToScreenMaterial->SetAmbientColor(RN::Color(_currentFadeColor.x, _currentFadeColor.y, _currentFadeColor.z, 1.0f));
	}

	void World::WillUpdate(float delta)
	{
		RN::Scene::WillUpdate(delta);
		UpdateForWindowSize();

		if(_vrCamera)
		{
			if(_vrCamera->GetHMDTrackingState().mode == RN::VRHMDTrackingState::Mode::Paused)
			{
				_isPaused = true;
			}
			else if(_vrCamera->GetHMDTrackingState().mode == RN::VRHMDTrackingState::Mode::Disconnected)
			{
				Exit();
			}
			else
			{
				_isPaused = false;
			}
		}

		if(RN::InputManager::GetSharedInstance()->IsControlToggling(RNCSTR("ESC")))
		{
			Exit();
		}

		if(_fadingIn || _fadingOut)
		{
			float fadeDelta = delta;
			if (fadeDelta > 0.03f) {
				fadeDelta = 0.03f;
			}
			_fadeTime -= fadeDelta;
			float percent = 1.0f;
			if (_fadeTime <= 0) {
				_fadeDone = true;
			}
			else {
				percent = 1.0f - _fadeTime / _fadeTimeTotal;
			}
			if (_fadingOut) {
				percent = 1.0f - percent;
			}
			if (_fadeDone) {
				_fadingIn = false;
				_fadingOut = false;
			}
			_currentFadeColor = RN::Vector3(percent);
		}
	}

	bool World::IsPlayerVisibleFrom(RN::Vector3 from) {
		RN::Vector3 playerPos = _player->GetWorldPosition();

		for (float x = -0.3f; x < 0.4f; x += 0.3f) {
			for (float y = 0.7f; y < 1.4f; y += 0.3f) {
				for (float z = -0.3f; z < 0.4f; z += 0.3f) {
					RN::Vector3 pos = playerPos + RN::Vector3(x, y, z);
					RN::PhysXContactInfo info = _physicsWorld->CastRay(from, pos, CollisionType::Level|CollisionType::Doors);
					if (info.distance < 0) {
						return true;
					}
				}
			}
		}
		return false;
	}

	void World::ShowUI(const RN::String *file) {
		HideUI();

		RN::Mesh *mesh = RN::Mesh::WithTexturedPlane(RN::Quaternion::WithEulerAngle(RN::Vector3(0, 90.0f, 0)), RN::Vector3(), RN::Vector2(0.64f, 0.64f));
		RN::Shader *vertexShader = RN::Renderer::GetActiveRenderer()->GetDefaultShader(RN::Shader::Type::Vertex, RN::Shader::Options::WithMesh(mesh));
		RN::Shader *fragmentShader = RN::Renderer::GetActiveRenderer()->GetDefaultShader(RN::Shader::Type::Fragment, RN::Shader::Options::WithMesh(mesh));
		RN::Material *material = RN::Material::WithShaders(vertexShader, fragmentShader);
		RN::Texture *texture = RN::Texture::WithName(file);
		material->AddTexture(texture);
		RN::Model *model = new RN::Model(mesh, material);
		_uiEntity = new RN::Entity(model);
		_uiEntity->SetPosition(RN::Vector3(0, 0.02f, -0.5f));
		_mainCamera->AddChild(_uiEntity);
		_uiPlayerPositionBackup = _player->GetWorldPosition();
		_uiPlayerRotationBackup = _mainCamera->GetWorldRotation();

		_player->SetWorldPosition(RN::Vector3(-1000, 0, 0));
		_mainCamera->SetWorldRotation(RN::Quaternion::WithIdentity());
	}

	void World::HideUI() {
		if (_uiEntity == nullptr) {
			return;
		}

		_mainCamera->RemoveChild(_uiEntity);
		_uiEntity = nullptr;
		_player->SetWorldPosition(_uiPlayerPositionBackup);
		_mainCamera->SetWorldRotation(_uiPlayerRotationBackup);
	}

	bool World::IsInUI() {
		return _uiEntity != nullptr;
	}

	void World::Fade(bool in, float seconds) {
		_fadingIn = in;
		_fadingOut = !in;
		_fadeTime = seconds;
		_fadeTimeTotal = seconds;
		_fadeDone = false;
	}

	bool World::IsFadeDone() {
		if (_fadeDone) {
			_fadeDone = false;
			return true;
		}
		return false;
	}
}
