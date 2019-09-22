#include <windows.h>
#include "application.h"

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE prevInstance, PSTR cmdLine, int shoeCmd)
{
	Application app;
	if (FAILED(app.Init(hInstance, true)))
		return 0;
	return app.EnterMsgLoop();
}