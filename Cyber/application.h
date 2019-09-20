#include <windows.h>
#include <d3dx9.h>
#include "mesh.h"

class Application
{
public:
	Application();
	~Application();
	HRESULT Init(HINSTANCE hAppIns, bool windowed);
	void Update(float deltaTime);
	void Render();
	void Cleanup();
	void Quit();

	void OnDeviceLost();
	void OnDeviceGained();

private:
	HWND m_hwnd;
	D3DPRESENT_PARAMETERS m_present;
	bool m_deviceLost;		//��ʾ�豸�Ƿ�ʧ
	Mesh m_soldier;
	float m_angle;
};