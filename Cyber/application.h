#include <d3dx9.h>
#include <fstream>
#include "mesh.h"
#include "skinnedMesh.h"
#include "Animation.h"
#include "OBB.h"

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
	ID3DXAnimationController *m_animController;

	Animation m_animation;
	float m_angle;

	OBB* m_pOBB1;
	OBB* m_pOBB2;
	D3DXQUATERNION m_rot1, m_rot2;
	float m_cdOfPressSpace;

	void Update(float deltaTime);
	void OnDeviceLost();
	void OnDeviceGained();
	void Render(float deltaTime);
	void RandomPlay4AnimFor4Model();
	void RandomBlend2Animation();
	void TestCallback();
	void Cleanup();
	void Quit();
};