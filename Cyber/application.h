#include <d3dx9.h>
#include "mesh.h"

class Application
{
public:
	Application();
	~Application();
	HRESULT Init(HINSTANCE hAppIns, bool windowed);
	void EnterMsgLoop();
private:
	HWND m_hwnd;
	D3DPRESENT_PARAMETERS m_present;
	bool m_deviceLost;		//��ʾ�豸�Ƿ�ʧ
	Mesh m_soldier;
	float m_angle;

	void Update(float deltaTime);
	void OnDeviceLost();
	void OnDeviceGained();
	void Render();
	void Cleanup();
	void Quit();
};