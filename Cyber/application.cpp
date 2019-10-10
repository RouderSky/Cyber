#include "application.h"

IDirect3DDevice9 *pD3DDevice = NULL;
ID3DXSprite *pSprite = NULL;
ID3DXFont *pText = NULL;
ID3DXLine *pLine = NULL;
ofstream streanOfDebug("debug.txt");

float m_show = 0.0f;
class CallbackHandler : public ID3DXAnimationCallbackHandler
{
public:
	HRESULT CALLBACK HandleCallback(THIS_ UINT Track, LPVOID pCallbackData)
	{
		m_show = 0.25f;
		return D3D_OK;
	}
};
CallbackHandler callbackHandler;

Application::Application()
{
	m_angle = 0.0f;
}

Application::~Application()
{
	if (pD3DDevice != NULL)
		pD3DDevice->Release();
	if (pText != NULL)
		pText->Release();
	if (pSprite != NULL)
		pSprite->Release();
	if (streanOfDebug.good())
		streanOfDebug.close();
	streanOfDebug << "Application Terminated \n";
}

//CALLBACKʲô��˼��
LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
	case WM_CREATE:
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	}

	return DefWindowProc(hwnd, msg, wParam, lParam);
}

HRESULT Application::Init(HINSTANCE hAppIns, bool windowed)
{
	streanOfDebug << "Applocation Started \n";

	//ע��һ��������
	WNDCLASS wc;
	memset(&wc, 0, sizeof(WNDCLASS));
	wc.style = CS_HREDRAW | CS_VREDRAW;		//CS_XXX����ʲô��
	wc.lpfnWndProc = WndProc;
	wc.hInstance = hAppIns;
	wc.lpszClassName = "D3DWND";
	RECT rc = { 0, 0, WINDOW_WIDTH, WINDOW_HEIGHT };
	AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, false);	//��
	RegisterClass(&wc);
	
	//����һ������ʵ��
	//��Ļ�ռ�ԭ�������Ͻ�
	m_hwnd = CreateWindow(
		"D3DWND",
		"Cyber",
		WS_OVERLAPPEDWINDOW,	//��
		0, 0,
		rc.right - rc.left,
		rc.bottom - rc.top,
		NULL,
		NULL,
		hAppIns,
		0);

	//�������
	//SetCursor(NULL);	//�������

	//��ʾ����
	ShowWindow(m_hwnd, SW_SHOW);
	UpdateWindow(m_hwnd);		//ʲô�����Ҫ��һ�£�

	//����d3d������
	IDirect3D9* d3d9 = Direct3DCreate9(D3D_SDK_VERSION);
	if (d3d9 == NULL)
	{
		streanOfDebug << "Direct3DCreate9() - FAILED \n";
		return E_FAIL;		//��
	}

	//�豸���ܼ��
	D3DCAPS9 caps;
	d3d9->GetDeviceCaps(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, &caps);
	if (caps.VertexShaderVersion < D3DVS_VERSION(2, 0) || caps.PixelShaderVersion < D3DPS_VERSION(2, 0))
		streanOfDebug << "Warning - Your graphic card does not support vertec and pixelshaders version 2.0 \n";

	int typeOfSupportedTransAndLight = 0;
	if (caps.DevCaps & D3DDEVCAPS_HWTRANSFORMANDLIGHT)
		typeOfSupportedTransAndLight = D3DCREATE_HARDWARE_VERTEXPROCESSING;
	else
		typeOfSupportedTransAndLight = D3DCREATE_SOFTWARE_VERTEXPROCESSING;

	//������ʾ�豸
	m_present.BackBufferWidth = WINDOW_WIDTH;
	m_present.BackBufferHeight = WINDOW_HEIGHT;
	m_present.BackBufferFormat = D3DFMT_A8R8G8B8;
	m_present.BackBufferCount = 2;
	m_present.MultiSampleType = D3DMULTISAMPLE_NONE;	//Ӧ���ǿ����
	m_present.MultiSampleQuality = 0;					//�����������
	m_present.SwapEffect = D3DSWAPEFFECT_DISCARD;		//��
	m_present.hDeviceWindow = m_hwnd;
	m_present.Windowed = windowed;
	m_present.EnableAutoDepthStencil = true;			//��
	m_present.AutoDepthStencilFormat = D3DFMT_D24S8;
	m_present.Flags = 0;								//��
	m_present.FullScreen_RefreshRateInHz = D3DPRESENT_RATE_DEFAULT;		//ˢ���ʣ�
	m_present.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;		//��
	if (FAILED(d3d9->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, m_hwnd, typeOfSupportedTransAndLight, &m_present, &pD3DDevice)))
	{
		streanOfDebug << "Failed to create IDirect3DDevice9 \n";
		return E_FAIL;
	}

	//�ͷ�d3d������
	d3d9->Release();

	//������ʾ�ı�
	D3DXCreateFont(pD3DDevice, 20, 0, FW_BOLD, 1, false,
		DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, DEFAULT_QUALITY,
		DEFAULT_PITCH | FF_DONTCARE, "Arial", &pText);

	//��
	D3DXCreateSprite(pD3DDevice, &pSprite);

	D3DXCreateLine(pD3DDevice, &pLine);

	//��������
	///�޹�������
	char *meshFileName = global::CombineStr(ROOT_PATH_TO_MESH, "soldier1.x");
	HRESULT hRes = m_soldier.Load(meshFileName, "Lighting.hlsl", "Shadow.hlsl");
	if (FAILED(hRes))
 		return E_FAIL;
	delete[]meshFileName;

	///�й�������
	meshFileName = global::CombineStr(ROOT_PATH_TO_MESH, "soldier2.x");
	hRes = m_drone.Load(meshFileName, "Lighting.hlsl", "Shadow.hlsl");
	delete[]meshFileName;
	if (FAILED(hRes))
		return E_FAIL;

	////��������
	srand(GetTickCount());
	//for (int i = 0; i < 4; i++)
	//{
	//	D3DXMATRIX mPos;
	//	D3DXMatrixTranslation(&mPos, -1.5f + i * 1.0f, 0.0f, 0.0f);
	//	m_positions.push_back(mPos);
	//	m_animControllers.push_back(m_drone.GetControllerCopy());
	//}
	//RandomPlay4AnimFor4Model();

	m_animController = m_drone.GetControllerCopy();
	//RandomBlend2Animation();

	TestCallback();

	m_animation.init();

	//��ʼ���������
	m_deviceLost = false;

	return S_OK;		//��
}

int Application::EnterMsgLoop()
{
	DWORD lastTime = GetTickCount();
	MSG msg;
	memset(&msg, 0, sizeof(MSG));
	while (msg.message != WM_QUIT)		//���յ��˳���Ϣѭ������Ϣʱ�˳�
	{
		if (PeekMessage(&msg, 0, 0, 0, PM_REMOVE))		//��Щ������ʲô��˼��
		{
			//��ϵͳ��Ϣ�ʹ���
			TranslateMessage(&msg);	//��
			DispatchMessage(&msg);	//��
		}
		else
		{
			//û��ϵͳ��Ϣ�͸��Լ�������
			DWORD curTime = GetTickCount();
			float deltaTime = (curTime - lastTime)*0.001f;
			Update(deltaTime);
			Render(deltaTime);
			lastTime = curTime;
		}
	}

	return (int)msg.wParam;
}

//������ʲô��ʧ�ˣ�
void Application::OnDeviceLost()
{
	try
	{
		//��Щ������Ҫ��һ��OnLostDevice��
		pText->OnLostDevice();
		pSprite->OnLostDevice();
		pLine->OnLostDevice();
		m_drone.OnLostDevice();
		m_animation.OnLostDevice();
		m_deviceLost = true;
	}
	catch (...)
	{
		streanOfDebug << "Error occured in Application::DeviceLost() \n";
	}
}

void Application::OnDeviceGained()
{
	try
	{
		pD3DDevice->Reset(&m_present);		//ʲôʱ����Ҫ����һ�£�
		//��Щ������Ҫ��һ��OnResetDevice��
		pText->OnResetDevice();
		pSprite->OnResetDevice();
		pLine->OnResetDevice();
		m_drone.OnResetDevice();
		m_animation.OnResetDevice();
		m_deviceLost = false;
	}
	catch (...)
	{
		streanOfDebug << "Error occured in Application::DeviceGained() \n";
	}
}

void Application::Update(float deltaTime)
{
	try
	{
		//�����ʾ�豸״̬
		HRESULT coop = pD3DDevice->TestCooperativeLevel();		//j...
		if (coop != D3D_OK)
		{
			if (coop == D3DERR_DEVICELOST)		//������ʲô������ʧ�ˣ�
			{
				OnDeviceLost();
			}
			else if (coop == D3DERR_DEVICENOTRESET)
			{
				OnDeviceGained();
			}
			Sleep(100);
			return;
		}

		//�������봦��
		if (global::KeyDown(VK_ESCAPE))
		{
			Quit();
		}

		if (global::KeyDown(VK_RETURN) && global::KeyDown(VK_MENU))		//ALT + RETURN
		{
			//�л����ܺ�����Release
			m_present.Windowed = !m_present.Windowed;

			//Ϊ��Ҫ�ֶ����ã�û��ͨ��TestCooperativeLevel��⵽��
			OnDeviceLost();
			OnDeviceGained();

			if (m_present.Windowed)
			{
				RECT rc = { 0, 0, WINDOW_WIDTH, WINDOW_HEIGHT };
				AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, false);
				SetWindowPos(m_hwnd, HWND_NOTOPMOST, 0, 0, rc.right - rc.left, rc.bottom - rc.top, SWP_SHOWWINDOW);		//��AdjustWindowRect��ʲô����
				UpdateWindow(m_hwnd);
			}
		}

		//�Զ����߼�
		m_angle += deltaTime;
		m_animation.Update(deltaTime);

		if (global::KeyDown(VK_RETURN))
		{
			Sleep(300);
			//RandomPlay4AnimFor4Model();
			RandomBlend2Animation();
		}

		if (m_show > 0.0f)
			m_show -= deltaTime;
	}
	catch (...)		//����ʲô�﷨��
	{
		streanOfDebug << "Error in Application::Update() \n";
	}
}

void Application::Render(float deltaTime)
{
	if (!m_deviceLost)
	{
		try
		{
			D3DXPLANE ground(0.0f, 1.0f, 0.0f, 0.0f);
			D3DXVECTOR4 lightPos(-20.0f, 75.0f, -120.0f, 0.0f);
			D3DXVECTOR4 lightColor(1.0f, 1.0f, 1.0f, 1.0f);

			D3DXMATRIX shadow;
			D3DXMatrixShadow(&shadow, &lightPos, &ground);

			D3DXMATRIX view;
			D3DXVECTOR3 targetPos(0.0f, 1.0f, 0.0f);
			D3DXVECTOR3 eyeDir(cos(m_angle), 1.0f, sin(m_angle));
			D3DXVECTOR3 eyePos = eyeDir * 2.0f;
			D3DXMatrixLookAtLH(&view, &eyePos, &targetPos, &D3DXVECTOR3(0.0f, 1.0f, 0.0f));
			
			D3DXMATRIX proj;
			D3DXMatrixPerspectiveFovLH(&proj, D3DX_PI / 4.0f, (float)WINDOW_WIDTH / (float)WINDOW_HEIGHT, NEAR_CLIP, FAR_CLIP);

			//��ʼ��������
			pD3DDevice->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, 0xffffffff, 1.0f, 0);
			if (SUCCEEDED(pD3DDevice->BeginScene()))
			{
				//m_soldier.Render(&view, &proj, &lightPos, &lightColor, &shadow);

				/*
				int numController = (int)m_animControllers.size();
				for (int i = 0; i < numController; i++)
				{
					m_animControllers[i]->AdvanceTime(deltaTime * 0.5, NULL);
					m_drone.UpdateMatrixOfBone2Model();
					m_drone.world = m_positions[i];
					//m_drone.SoftRender(&view, &proj, &lightPos, &lightColor, &shadow);
					m_drone.HardRender(&view, &proj, &lightPos, &lightColor, &shadow);
					//m_drone.RenderSkeleton(&view, &proj);
				}
				*/

				m_animController->AdvanceTime(deltaTime * 0.5, &callbackHandler);
				m_drone.UpdateMatrixOfBone2Model();
				m_drone.HardRender(&view, &proj, &lightPos, &lightColor, &shadow);
				global::ShowAllTrackStatus(m_animController);

				if (m_show>0.0f)
				{
					RECT rc = { 0,0,WINDOW_WIDTH,WINDOW_HEIGHT };
					pText->DrawText(NULL, "BANG!", -1, &rc, DT_CENTER | DT_VCENTER | DT_NOCLIP, 0xFF000000);
					SetRect(&rc, -5, -5, WINDOW_WIDTH-5, WINDOW_HEIGHT-5);
					pText->DrawText(NULL, "BANG!", -1, &rc, DT_CENTER | DT_VCENTER | DT_NOCLIP, 0xFFFFFF00);
				}

				//m_animation.Draw();

				//UI
				RECT rc = { 10,10,0,0 };
				pText->DrawText(NULL, "Press Return to do something", -1, &rc, DT_LEFT | DT_TOP | DT_NOCLIP, 0x66000000);
				pD3DDevice->EndScene();
				pD3DDevice->Present(NULL, NULL, NULL, NULL);
			}
		}
		catch (...)
		{
			streanOfDebug << "Error in Application::Render() \n";
		}
	}
}

void Application::RandomPlay4AnimFor4Model()
{
	int numAnimControllers = (int)m_animControllers.size();
	for (int i = 0; i < numAnimControllers; i++)
	{
		int numAnimations = m_animControllers[i]->GetMaxNumAnimationSets();
		ID3DXAnimationSet *anim = NULL;
		m_animControllers[i]->GetAnimationSet(rand() % numAnimations, &anim);
		m_animControllers[i]->SetTrackAnimationSet(0, anim);
		anim->Release();
	}
}

void Application::RandomBlend2Animation()
{
	m_animController->ResetTime();

	//�����ȡ�������������ŵ����������
	int numAnimations = m_animController->GetMaxNumAnimationSets();
	ID3DXAnimationSet* anim1 = NULL;
	ID3DXAnimationSet* anim2 = NULL;
	m_animController->GetAnimationSet(rand() % numAnimations, &anim1);
	m_animController->GetAnimationSet(rand() % numAnimations, &anim2);
	m_animController->SetTrackAnimationSet(0, anim1);
	m_animController->SetTrackAnimationSet(1, anim2);

	//���
	float w = (rand() % 1000) / 1000.0f;
	m_animController->SetTrackWeight(0, w);
	m_animController->SetTrackWeight(1, 1.0f - w);		//todo�����Կ��ǲ����ܺͲ�Ϊ1Ҳ����
	m_animController->SetTrackSpeed(0, (rand() % 1000) / 500.0f);
	m_animController->SetTrackSpeed(1, (rand() % 1000) / 500.0f);
	m_animController->SetTrackPriority(0, D3DXPRIORITY_HIGH);		//todo���ĵ����ȼ�����
	m_animController->SetTrackPriority(1, D3DXPRIORITY_HIGH);
	m_animController->SetTrackEnable(0, true);		//todo��ȥ�����ԣ�Ӧ����Ĭ�Ͽ�����
	m_animController->SetTrackEnable(1, true);
}

void Application::TestCallback()
{
	//�����һ������
	ID3DXKeyframedAnimationSet* animSet = NULL;
	m_animController->GetAnimationSet(1, (ID3DXAnimationSet**)&animSet);

	//ѹ������
	ID3DXBuffer* compressedData = NULL;
	animSet->Compress(D3DXCOMPRESS_DEFAULT, 0.5f, NULL, &compressedData);

	//����callback
	const UINT numCallbacks = 1;
	D3DXKEY_CALLBACK keys[numCallbacks];
	double ticksPerSecond = animSet->GetSourceTicksPerSecond();
	keys[0].Time = float(animSet->GetPeriod() / 2.0f*ticksPerSecond);
	keys[0].pCallbackData = (void*)&m_drone;
	ID3DXCompressedAnimationSet* compressedAnimSet = NULL;
	//���callbackһ��Ҫѹ������
	D3DXCreateCompressedAnimationSet(
		animSet->GetName(),
		animSet->GetSourceTicksPerSecond(),
		animSet->GetPlaybackType(),
		compressedData,
		numCallbacks,
		keys,
		&compressedAnimSet);
	compressedData->Release();

	m_animController->UnregisterAnimationSet(animSet);			//�ɲ��������ɾ����
	m_animController->RegisterAnimationSet(compressedAnimSet);
	m_animController->SetTrackAnimationSet(0, compressedAnimSet);
	compressedAnimSet->Release();
}

void Application::Quit()
{
	DestroyWindow(m_hwnd);	//�رմ���
	PostQuitMessage(0);		//��ֹ��Ϣѭ��
}