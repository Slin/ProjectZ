#include <Rayne.h>
#include "SGApplication.h"

#if RN_BUILD_RELEASE
#pragma comment(linker, "/SUBSYSTEM:windows /ENTRY:mainCRTStartup")
#endif

int main(int argc, const char *argv[])
{
#if RN_BUILD_DEBUG
//	_CrtSetDbgFlag(_CRTDBG_CHECK_ALWAYS_DF);
#endif

	RN::Initialize(argc, argv, new SG::Application());
}
