//
//  PZApplication.cpp
//  Project: Z
//
//  Copyright 2018 by Überpixel. All rights reserved.
//  Unauthorized use is punishable by torture, mutilation, and vivisection.
//

#include "PZApplication.h"
#include "PZWorld.h"

#if RN_PLATFORM_WINDOWS
#include "RNOculusWindow.h"
#endif
#if !defined(BUILD_FOR_OCULUS)
#include "RNOpenVRWindow.h"
#endif

namespace PZ
{
	Application::Application() : _vrWindow(nullptr), _window(nullptr)
	{
#if RN_PLATFORM_WINDOWS
		ShowCursor(false);
#endif
	}

	Application::~Application()
	{
		SafeRelease(_vrWindow);
		SafeRelease(_window);
	}

	void Application::SetupVR()
	{ 
		if (_vrWindow)
			return;

#if defined(BUILD_FOR_OCULUS)
		_vrWindow = new RN::OculusWindow();
#else
#if RN_PLATFORM_WINDOWS
		if (RN::Kernel::GetSharedInstance()->GetArguments().HasArgument("openvr", 0))
		{
			_vrWindow = new RN::OpenVRWindow();
		}
		else if (RN::Kernel::GetSharedInstance()->GetArguments().HasArgument("oculusvr", 0))
		{
			_vrWindow = new RN::OculusWindow();
		}
		else
		{
			RNDebug("Either --openvr or --oculusvr must be set as start parameter!");
			World::Exit();
		}
#else
		_vrWindow = new RN::OpenVRWindow();
#endif
#endif
	}

	void Application::SetupPreviewWindow()
	{
		if (_window)
			return;

		RN::Renderer *renderer = RN::Renderer::GetActiveRenderer();
		_window = renderer->CreateAWindow(RN::Vector2(960, 540), RN::Screen::GetMainScreen());
		_window->SetTitle(GetTitle());
		_window->Show();

		if(_vrWindow)
			_vrWindow->PreparePreviewWindow(_window);
	}

	RN::RenderingDevice *Application::GetPreferredRenderingDevice(const RN::Array *devices) const
	{
		RN::RenderingDevice *preferred = nullptr;

		if(_vrWindow)
		{
			RN::RenderingDevice *vrDevice = _vrWindow->GetOutputDevice();
			if (vrDevice)
			{
				devices->Enumerate<RN::RenderingDevice>([&](RN::RenderingDevice *device, size_t index, bool &stop) {
					if (vrDevice->IsEqual(device))
					{
						preferred = device;
						stop = true;
					}
				});
			}
		}

		if (!preferred)
			preferred = RN::Application::GetPreferredRenderingDevice(devices);

		return preferred;
	}

	void Application::WillFinishLaunching(RN::Kernel *kernel)
	{
//		SetupVR();

		RN::Application::WillFinishLaunching(kernel);
		RN::Shader::Sampler::SetDefaultAnisotropy(16);
	}

	void Application::DidFinishLaunching(RN::Kernel *kernel)
	{
		if(_vrWindow)
		{
			RN::Renderer *renderer = RN::Renderer::GetActiveRenderer();
			renderer->SetMainWindow(_vrWindow);

			RN::Window::SwapChainDescriptor swapChainDescriptor;
#if defined(BUILD_FOR_OCULUS)
			swapChainDescriptor.depthStencilFormat = RN::Texture::Format::Depth32F;
#endif
			if (RN::Kernel::GetSharedInstance()->GetArguments().HasArgument("oculusvr", 0))
			{
				swapChainDescriptor.depthStencilFormat = RN::Texture::Format::Depth32F;
			}

			_vrWindow->StartRendering(swapChainDescriptor);
		}
		else
		{
//			RN::Kernel::GetSharedInstance()->SetMaxFPS(60);
		}

		RN::Application::DidFinishLaunching(kernel);

#if RN_PLATFORM_MAC_OS
		if(!_vrWindow)
		{
			SetupPreviewWindow();
		}
		World *world = new World(_vrWindow, _window, false, 4, false);
#else
		SetupPreviewWindow();
		World *world = new World(_vrWindow, _window, true, 8, false);
#endif

		RN::SceneManager::GetSharedInstance()->AddScene(world);
	}
}
