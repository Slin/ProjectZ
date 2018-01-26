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

	World::World(RN::VRWindow *vrWindow, RN::Window *window, bool hasShadows, RN::uint8 msaa, bool debug) : _window(nullptr), _vrWindow(nullptr), _hasShadows(hasShadows), _msaa(msaa), _debug(debug), _audioWorld(nullptr), _vrCamera(nullptr), _isPaused(false), _shadowCamera(nullptr)
	{
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

		if(_vrCamera)
		{
			_vrCamera->SetClipFar(1500.0f);

			_shadowCamera = new RN::Camera();
			_shadowCamera->SetFlags(_shadowCamera->GetFlags() | RN::Camera::Flags::NoRender);
			_vrCamera->GetHead()->AddChild(_shadowCamera->Autorelease());
			AddNode(_vrCamera);
		}

		CreateTestLevel();
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
			RN::PostProcessingAPIStage *copyPass = new RN::PostProcessingAPIStage(RN::PostProcessingAPIStage::Type::Convert);
			resolvePass->AddRenderPass(copyPass);

			RN::Camera *camera = new RN::Camera();
			camera->GetRenderPass()->SetFramebuffer(msaaFramebuffer);
			camera->GetRenderPass()->AddRenderPass(resolvePass);
			
			_shadowCamera = camera;
			AddNode(camera);

			camera->SetPosition(RN::Vector3(185.0, 40.0, -183.0));
			camera->SetRotation(RN::Vector3(0.0f, -90.0f, 0.0f));
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
			_audioWorld = new RN::SteamAudioWorld(foundOutputAudioDevice);
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

//		RN::Model *groundModel = RN::Model::WithName(RNCSTR("models/levels/castle/castle.sgm"));
//		RN::Entity *level = new RN::Entity(groundModel);
//		AddNode(level->Autorelease());

		//TODO: Mesh braucht eine optionale kopie im RAM, damit das trianglemeshshape korrekt funktioniert
//		RN::PhysXMaterial *groundPhysicsMaterial = new RN::PhysXMaterial();
/*#if LEVEL_COLLISION_CHANGED == 1
		RN::NewtonTriangleMeshShape *levelShape = RN::NewtonTriangleMeshShape::WithModel(groundModel);
		levelShape->SerializeToFile(RNCSTR("level_collision.newton"));
#else
		RN::NewtonShape *levelShape = RN::NewtonTriangleMeshShape::WithFile(RNCSTR("level_collision.newton"));
#endif
		RN::NewtonRigidBody *groundBody = RN::NewtonRigidBody::WithShape(levelShape, 0.0f);
		groundBody->SetCollisionFilter(CollisionType::Level, CollisionType::All);
		level->AddAttachment(groundBody);*/

/*		if(_audioWorld)
		{
			RN::SteamAudioMaterial groundAudioMaterial;
			groundAudioMaterial.lowFrequencyAbsorption = 0.2f;
			groundAudioMaterial.midFrequencyAbsorption = 0.3f;
			groundAudioMaterial.highFrequencyAbsorption = 0.4f;
			groundAudioMaterial.scattering = 0.5f;
			_audioWorld->AddMaterial(groundAudioMaterial);

			RN::SteamAudioMaterial wallAudioMaterial;
			wallAudioMaterial.lowFrequencyAbsorption = 0.03f;
			wallAudioMaterial.midFrequencyAbsorption = 0.07f;
			wallAudioMaterial.highFrequencyAbsorption = 0.1f;
			wallAudioMaterial.scattering = 0.3f;
			_audioWorld->AddMaterial(wallAudioMaterial);

			RN::Model::LODStage *lodStage = groundModel->GetLODStage(0);
			for(int i = 0; i < lodStage->GetCount(); i++)
			{
				RN::Material *material = lodStage->GetMaterialAtIndex(i);
				material->SetAlphaToCoverage(true, 0.0, 1.0);
				material->SetCullMode(RN::CullMode::None);

				RN::SteamAudioGeometry groundAudioGeometry;
				groundAudioGeometry.mesh = lodStage->GetMeshAtIndex(i);
				groundAudioGeometry.materialIndex = (i == 0) ? 0 : 1;
				groundAudioGeometry.position = level->GetWorldPosition();
				groundAudioGeometry.scale = level->GetWorldScale();
				groundAudioGeometry.rotation = level->GetWorldRotation();
				_audioWorld->AddStaticGeometry(groundAudioGeometry);
			}

			_audioWorld->UpdateScene();*/

/*			if(_isClient)
			{
				RN::Model *radioModel = RN::Model::WithName(RNCSTR("models/utilities/radio.sgm"));
				RN::AudioAsset *audioAsset = RN::AudioAsset::WithName(RNCSTR("audio/neverstop.ogg"));
				for(int i = 0; i < 1; i++)
				{
					RN::SteamAudioSource *musicSource = new RN::SteamAudioSource(audioAsset, true);
					musicSource->Play();
					musicSource->SetRepeat(true);
					musicSource->SetRadius(0.5f);
					musicSource->SetGain(0.1f);
					RN::Entity *radioEntity = new RN::Entity(radioModel);
					radioEntity->AddChild(musicSource->Autorelease());
					radioEntity->SetWorldPosition(/*RN::Vector3(RN::RandomNumberGenerator::GetSharedGenerator()->GetRandomFloatRange(-30.0f, 30.0f), 1.0f, RN::RandomNumberGenerator::GetSharedGenerator()->GetRandomFloatRange(-30.0f, 30.0f))*//*RN::Vector3(185.0, 1.0, -175.0));
					radioEntity->SetRotation(RN::Vector3(135.0f, 0.0f, 0.0f));
					AddNode(radioEntity);
				}
			}*/
//		}
	}

	void World::UpdateForWindowSize() const
	{
		if(!_vrCamera || !_window)
			return;

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
	}
}
