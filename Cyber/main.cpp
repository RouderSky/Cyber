#include <windows.h>
#include "application.h"

//һ��������Ӧ����������
//���˲²⣺
//frame 1<--->* container
//frame 1<--->* ����

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE prevInstance, PSTR cmdLine, int shoeCmd)
{
	Application app;
	if (FAILED(app.Init(hInstance, true)))
		return 0;
	return app.EnterMsgLoop();
}