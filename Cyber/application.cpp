#include "application.h"

IDirect3DDevice9 *pD3DDevice = NULL;
ID3DXSprite *pSprite = NULL;
ID3DXFont *pText = NULL;
ID3DXEffect *pLambertDiffuseEffect = NULL;
ID3DXEffect *pShadowEffect = NULL;
ofstream streanOfDebug("debug.txt");

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
	if (pLambertDiffuseEffect != NULL)
		pLambertDiffuseEffect->Release();
	if (pShadowEffect != NULL)
		pShadowEffect->Release();

	streanOfDebug << "Application Terminated \n";

	if (streanOfDebug.good())
		streanOfDebug.close();
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

	//����shader�ļ�
	ID3DXBuffer *pErrorMsgs = NULL;
	char *effectFileName = global::CombineStr(ROOT_PATH_TO_EFFECT, "LambertDiffuse.hlsl");
	HRESULT hRes = D3DXCreateEffectFromFile(
		pD3DDevice, 
		effectFileName, 
		NULL, 
		NULL,
		D3DXSHADER_DEBUG,
		NULL,
		&pLambertDiffuseEffect,
		&pErrorMsgs);
	//delete effectFileName;		//Ϊɶdelete�ᱨ��
	if (FAILED(hRes) && (pErrorMsgs != NULL))
	{
		MessageBox(NULL, (char*)pErrorMsgs->GetBufferPointer(), "Load LambertDiffuse Effect Error", MB_OK);		//MB_OK��ɶ��
		return E_FAIL;
	}

	effectFileName = global::CombineStr(ROOT_PATH_TO_EFFECT, "Shadow.hlsl");
	hRes = D3DXCreateEffectFromFile(
		pD3DDevice,
		effectFileName,
		NULL,
		NULL,
		D3DXSHADER_DEBUG,
		NULL,
		&pShadowEffect,
		&pErrorMsgs);
	//delete effectFileName;
	if (FAILED(hRes) && (pErrorMsgs != NULL))
	{
		MessageBox(NULL, (char*)pErrorMsgs->GetBufferPointer(), "Load Shadow Effect Error", MB_OK);		//MB_OK��ɶ��
		return E_FAIL;
	}

	//��������
	char *meshFileName = global::CombineStr(ROOT_PATH_TO_MESH, "soldier.x");
	hRes = m_soldier.Load(meshFileName);
	if (FAILED(hRes))
	{
		MessageBox(NULL, "Error", "Load Mesh Error", MB_OK);
		return E_FAIL;
	}
	//delete meshFileName;

	//��ʼ���������
	m_deviceLost = false;

	return S_OK;		//��
}

void Application::EnterMsgLoop()
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
			Render();
			lastTime = curTime;
		}
	}
}

//������ʲô��ʧ�ˣ�
void Application::OnDeviceLost()
{
	try
	{
		//��Щ������Ҫ��һ��OnLostDevice��
		pText->OnLostDevice();
		pSprite->OnLostDevice();
		pLambertDiffuseEffect->OnLostDevice();
		pShadowEffect->OnLostDevice();
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
		pLambertDiffuseEffect->OnResetDevice();
		pShadowEffect->OnResetDevice();
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
	}
	catch (...)		//... ��ʲô�﷨��
	{
		streanOfDebug << "Error in Application::Update() \n";
	}
}

void Application::Render()
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

			D3DXMATRIX world;
			D3DXMatrixIdentity(&world);

			D3DXMATRIX view;
			D3DXVECTOR3 targetPos(0.0f, 1.0f, 0.0f);		//Ӧ�ø���world��λ��������Ŷ�...
			D3DXVECTOR3 eyeDir(cos(m_angle), 1.0f, sin(m_angle));
			float distanceFromEye2Tar = 2.0;
			D3DXVECTOR3 eyePos = targetPos + eyeDir * distanceFromEye2Tar;
			D3DXMatrixLookAtRH(&view, &eyePos, &targetPos, &D3DXVECTOR3(0.0f, 1.0f, 0.0f));
			
			D3DXMATRIX proj;
			D3DXMatrixPerspectiveFovLH(&proj, D3DX_PI / 4.0f, (float)WINDOW_WIDTH / (float)WINDOW_HEIGHT, NEAR_CLIP, FAR_CLIP);
		
			//�����������Ҳ���԰ɣ�
			pD3DDevice->SetTransform(D3DTS_WORLD, &world);
			pD3DDevice->SetTransform(D3DTS_VIEW, &view);
			pD3DDevice->SetTransform(D3DTS_PROJECTION, &proj);
			//////////////////////////////////////////////////////////

			//��ʼ��������
			pD3DDevice->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, 0xffffffff, 1.0f, 0);
			if (SUCCEEDED(pD3DDevice->BeginScene()))
			{
				pLambertDiffuseEffect->SetMatrix("matW", &world);
				pLambertDiffuseEffect->SetMatrix("matVP", &(view * proj));
				pLambertDiffuseEffect->SetVector("lightPos", &lightPos);
				pLambertDiffuseEffect->SetVector("lightColor", &lightColor);
				D3DXHANDLE hTech = pLambertDiffuseEffect->GetTechniqueByName("LambertDiffuse");
				pLambertDiffuseEffect->SetTechnique(hTech);
				UINT passCont;
				pLambertDiffuseEffect->Begin(&passCont, NULL);
				for (UINT i = 0; i < passCont; i++)
				{
					pLambertDiffuseEffect->BeginPass(i);
					m_soldier.Render();
					pLambertDiffuseEffect->EndPass();
				}
				pLambertDiffuseEffect->End();

				pShadowEffect->SetMatrix("matW", &shadow);
				pShadowEffect->SetMatrix("matVP", &(view * proj));
				hTech = pShadowEffect->GetTechniqueByName("Shadow");
				pShadowEffect->SetTechnique(hTech);
				pShadowEffect->Begin(&passCont, NULL);
				for (UINT i = 0; i < passCont; i++)
				{
					pShadowEffect->BeginPass(i);
					m_soldier.Render();
					pShadowEffect->EndPass();
				}
				pShadowEffect->End();

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

void Application::Quit()
{
	DestroyWindow(m_hwnd);	//�رմ���
	PostQuitMessage(0);		//��ֹ��Ϣѭ��
}