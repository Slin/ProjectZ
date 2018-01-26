//
//  PZApplication.h
//  Project Z
//
//  Copyright 2018 by Überpixel. All rights reserved.
//  Unauthorized use is punishable by torture, mutilation, and vivisection.
//

#ifndef __PROJECT_Z_APPLICATION_H_
#define __PROJECT_Z_APPLICATION_H_

#include <Rayne.h>

using namespace RN::numeric;

namespace RN
{
	class VRWindow;
}

namespace PZ
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
	};
}


#endif /* __PROJECT_Z_APPLICATION_H_ */
