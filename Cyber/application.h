#include <d3dx9.h>
#include "mesh.h"
#include "skinnedMesh.h"
#include "Animation.h"

class Application
{
public:
	Application();
	~Application();
	HRESULT Init(HINSTANCE hAppIns, bool windowed);
	int EnterMsgLoop();
private:
	HWND m_hwnd;
	D3DPRESENT_PARAMETERS m_present;
	bool m_deviceLost;		//显示设备是否丢失
	Mesh m_soldier;

	SkinnedMesh m_drone;
	vector<ID3DXAnimationController*> m_animControllers;
	vector<D3DXMATRIX> m_positions;

	Animation m_animation;
	float m_angle;

	void Update(float deltaTime);
	void OnDeviceLost();
	void OnDeviceGained();
	void Render(float deltaTime);
	void RandomizeAnimations();
	void Cleanup();
	void Quit();
};