//
//  SGWorld.cpp
//  Sword Game
//
//  Copyright 2017 by Überpixel. All rights reserved.
//  Unauthorized use is punishable by torture, mutilation, and vivisection.
//

#define LEVEL_COLLISION_CHANGED 0

#include "SGWorld.h"
#include "../../../Rayne/Modules/PhysX/RNPhysXShape.h"

#if RN_PLATFORM_WINDOWS
#include "RNOculusWindow.h"
#endif

namespace SG
{
	World *World::_sharedInstance = nullptr;

	World *World::GetSharedInstance()
	{
		return _sharedInstance;
	}

	void World::Exit()
	{
		Platform::Shutdown();
		google::protobuf::ShutdownProtobufLibrary();
		GetSharedInstance()->RemoveAttachment(GetSharedInstance()->GetPhysicsWorld());
		GetSharedInstance()->GetPhysicsWorld()->Release();
		exit(0);
		//RN::Kernel::GetSharedInstance()->Exit();
	}

	World::World(bool isClient, bool isServer, RN::VRWindow *vrWindow, RN::Window *window, bool hasShadows, RN::uint8 msaa, bool debug) : _isClient(isClient), _isServer(isServer), _window(nullptr), _vrWindow(nullptr), _hasShadows(hasShadows), _msaa(msaa), _debug(debug), _audioWorld(nullptr), _networkWorld(nullptr), _networkHost(nullptr), _vrCamera(nullptr), _speech(nullptr), _isPaused(false), _shadowCamera(nullptr)
	{
		GOOGLE_PROTOBUF_VERIFY_VERSION;

		Platform::Initialize();

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

		_physicsWorld = new RN::NewtonWorld(RN::Vector3(0.0, -9.81, 0.0), true);
		_physicsWorld->SetSubsteps(4);
		AddAttachment(_physicsWorld);

		if(_vrCamera)
		{
			_vrCamera->SetClipFar(1500.0f);

			_shadowCamera = new RN::Camera();
			_shadowCamera->SetFlags(_shadowCamera->GetFlags() | RN::Camera::Flags::NoRender);
			_vrCamera->GetHead()->AddChild(_shadowCamera->Autorelease());
			AddNode(_vrCamera);
		}

		InitializeNetworking();
		CreateTestLevel();

		if(_isServer)
		{
			ObjectManager::CreateSceneNodeForType(0, SGNetworking::ObjectState::Type::ObjectState_Type_DUMMY, RN::Vector3(185.0, 0.0, -180.0));

			for(int i = 0; i < 5; i++)
			{
				SGNetworking::ObjectState_Type type = static_cast<SGNetworking::ObjectState_Type>((i % 5) + 2);
				ObjectManager::CreateSceneNodeForType(0, type, RN::RandomNumberGenerator::GetSharedGenerator()->GetRandomVector3Range(RN::Vector3(180.0f, 0.3f, -180.0f), RN::Vector3(190.0f, 0.5f, -170.0f)));
			}

			if(_isClient)
			{
				ObjectManager::CreateSceneNodeForType(0, SGNetworking::ObjectState::Type::ObjectState_Type_PLAYER, RN::Vector3(185.0, 0.0, -183.0));
			}
		}
	}

	void World::InitializeNetworking()
	{
		if(_isServer || _isClient)
		{
			_networkWorld = new RN::ENetWorld();
			AddAttachment(_networkWorld);

			RN::uint32 port = 12345;
			if(RN::Kernel::GetSharedInstance()->GetArguments().HasArgumentAndValue("port", 0))
			{
				RN::ArgumentParser::Argument argument = RN::Kernel::GetSharedInstance()->GetArguments().ParseArgument("port", 0);
				RN::String *portString = argument.GetValue();
				port = strtol(portString->GetUTF8String(), nullptr, 10);
			}

			RN::String *ip = RNCSTR("localhost");
			if(RN::Kernel::GetSharedInstance()->GetArguments().HasArgumentAndValue("ip", 0))
			{
				RN::ArgumentParser::Argument argument = RN::Kernel::GetSharedInstance()->GetArguments().ParseArgument("ip", 0);
				ip = argument.GetValue();
			}

			if(_isServer)
			{
				_networkHost = new Server(port);
			}
			else
			{
				Client *client = new Client();
				client->Connect(ip, port);
				_networkHost = client;
			}
		}
	}

	void World::InitializePlatform()
	{
		if(_isClient)
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
		RN::SteamAudioDevice *foundInputAudioDevice = nullptr;

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
					}
					if(audioDevice->id->IsEqual(oculusInputAudioDeviceID))
					{
						foundInputAudioDevice = audioDevice->Retain();
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

		if(!foundInputAudioDevice)
		{
			foundInputAudioDevice = RN::SteamAudioWorld::GetDefaultInputDevice();
		}

		if(!_isClient)
			foundInputAudioDevice = nullptr;

		if(foundOutputAudioDevice)
		{
			_audioWorld = new RN::SteamAudioWorld(foundOutputAudioDevice);
			AddAttachment(_audioWorld);

			if(_vrCamera)
				_audioWorld->SetListener(_vrCamera->GetHead());
			else if(_shadowCamera)
				_audioWorld->SetListener(_shadowCamera);
		}

		if(foundOutputAudioDevice && foundInputAudioDevice)
		{
			_speech = new Speech(foundInputAudioDevice);
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

		RN::Model *groundModel = RN::Model::WithName(RNCSTR("models/levels/castle/castle.sgm"));
		_ground = new RN::Entity(groundModel);
		AddNode(_ground->Autorelease());

		//TODO: Mesh braucht eine optionale kopie im RAM, damit das trianglemeshshape korrekt funktioniert
//		RN::PhysXMaterial *groundPhysicsMaterial = new RN::PhysXMaterial();
#if LEVEL_COLLISION_CHANGED == 1
		RN::NewtonTriangleMeshShape *levelShape = RN::NewtonTriangleMeshShape::WithModel(groundModel);
		levelShape->SerializeToFile(RNCSTR("level_collision.newton"));
#else
		RN::NewtonShape *levelShape = RN::NewtonTriangleMeshShape::WithFile(RNCSTR("level_collision.newton"));
#endif
		RN::NewtonRigidBody *groundBody = RN::NewtonRigidBody::WithShape(levelShape, 0.0f);
		groundBody->SetCollisionFilter(CollisionType::Level, CollisionType::All);
		_ground->AddAttachment(groundBody);

		if(_audioWorld)
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
				groundAudioGeometry.position = _ground->GetWorldPosition();
				groundAudioGeometry.scale = _ground->GetWorldScale();
				groundAudioGeometry.rotation = _ground->GetWorldRotation();
				_audioWorld->AddStaticGeometry(groundAudioGeometry);
			}

			_audioWorld->UpdateScene();

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
		}

		//DRAGON!!!!
		RN::Model *dragonModel = RN::Model::WithName(RNCSTR("models/creatures/dragon.sgm"));
		RN::Entity *dragon = new RN::Entity(dragonModel);
		AddNode(dragon->Autorelease());
		dragon->SetWorldPosition(RN::Vector3(185.0, 0.0, -183.0+40.0f));
		dragon->SetWorldRotation(RN::Vector3(30.0f, 0.0, 0.0));

		RN::Entity *skyEntity = new RN::Entity(RN::Model::WithSkycube(RNCSTR("models/skies/sky_left.png"), RNCSTR("models/skies/sky_front.png"), RNCSTR("models/skies/sky_right.png"), RNCSTR("models/skies/sky_back.png"), RNCSTR("models/skies/sky_up.png"), RNCSTR("models/skies/sky_down.png")));
		skyEntity->SetScale(RN::Vector3(50000.0f));
		AddNode(skyEntity->Autorelease());
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

		Platform::HandleEvents();

		if(_speech)
		{
			_speech->Update(delta);
		}

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
		
		if(GetIsServer())
		{
			bool didSpawnSomething = false;
			if(RN::InputManager::GetSharedInstance()->IsControlToggling(RNCSTR("1")))
			{
				didSpawnSomething = true;
				if(!_didSpawnSomething)
				{
					ObjectManager::CreateSceneNodeForType(0, SGNetworking::ObjectState::Type::ObjectState_Type_SWORD, RN::Vector3(185.0f, 0.5f, -185.0f));
				}
			}
			
			if(RN::InputManager::GetSharedInstance()->IsControlToggling(RNCSTR("2")))
			{
				didSpawnSomething = true;
				if(!_didSpawnSomething)
				{
					ObjectManager::CreateSceneNodeForType(0, SGNetworking::ObjectState::Type::ObjectState_Type_SHIELD, RN::Vector3(185.0f, 0.5f, -185.0f));
				}
			}
			
			_didSpawnSomething = didSpawnSomething;
		}
	}
}
