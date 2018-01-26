//
//  SGApplication.h
//  Sword Game
//
//  Copyright 2017 by Überpixel. All rights reserved.
//  Unauthorized use is punishable by torture, mutilation, and vivisection.
//

#ifndef __SWORD_GAME_APPLICATION_H_
#define __SWORD_GAME_APPLICATION_H_

#include <Rayne.h>

using namespace RN::numeric;

namespace RN
{
	class VRWindow;
}

namespace SG
{
	class Application : public RN::Application
	{
	public:
		Application();
		~Application();

		RN::RenderingDevice *GetPreferredRenderingDevice(const RN::Array *devices) const final;

		void WillFinishLaunching(RN::Kernel *kernel) override;
		void DidFinishLaunching(RN::Kernel *kernel) override;

		RN::VRWindow *GetVRWindow() const { return _vrWindow; }

	private:
		void SetupVR();
		void SetupPreviewWindow();
		RN::VRWindow *_vrWindow;
		RN::Window *_window;
		bool _isClient;
		bool _isServer;
	};
}


#endif /* __SWORD_GAME_APPLICATION_H_ */
