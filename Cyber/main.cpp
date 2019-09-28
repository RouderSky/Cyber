#include <windows.h>
#include "application.h"

//一个骨骼对应若干子网格
//个人猜测：
//frame 1<--->* container
//frame 1<--->* 网格

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE prevInstance, PSTR cmdLine, int shoeCmd)
{
	Application app;
	if (FAILED(app.Init(hInstance, true)))
		return 0;
	return app.EnterMsgLoop();
}