#include "application.h"

Application::Application()
{
	m_angle = 0.0f;
}

Application::~Application()
{
	global::Release();
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
	global::streanOfDebug << "Applocation Started \n";

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
		global::streanOfDebug << "Direct3DCreate9() - FAILED \n";
		return E_FAIL;
	}

	//�豸���ܼ��
	D3DCAPS9 caps;
	d3d9->GetDeviceCaps(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, &caps);
	if (caps.VertexShaderVersion < D3DVS_VERSION(2, 0) || caps.PixelShaderVersion < D3DPS_VERSION(2, 0))
		global::streanOfDebug << "Warning - Your graphic card does not support vertec and pixelshaders version 2.0 \n";

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
	if (FAILED(d3d9->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, m_hwnd, typeOfSupportedTransAndLight, &m_present, &global::pDevice)))
	{
		global::streanOfDebug << "Failed to create IDirect3DDevice9 \n";
		return E_FAIL;
	}

	//�ͷ�d3d������
	d3d9->Release();

	//������ʾ�ı�
	D3DXCreateFont(global::pDevice, 20, 0, FW_BOLD, 1, false,
		DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, DEFAULT_QUALITY,
		DEFAULT_PITCH | FF_DONTCARE, "Arial", &global::pText);

	//��
	D3DXCreateSprite(global::pDevice, &global::pSprite);

	//����shader�ļ�
	ID3DXBuffer *pErrorMsgs = NULL;
	char *effectFileName = global::CombineStr(ROOT_PATH_TO_EFFECT, "lightting.fx");
	HRESULT hRes = D3DXCreateEffectFromFile(global::pDevice, effectFileName);
	delete effectFileName;
	if (FAILED(hRes) && (pErrorMsgs != NULL))
	{
		MessageBox(NULL, (char*)pErrorMsgs->GetBufferPointer(), "Load Effect Error", MB_OK);		//MB_OK��ɶ��
		return E_FAIL;		//��
	}

	//��������
	char *meshFileName = global::CombineStr(ROOT_PATH_TO_MESH, "soldier.x");
	hRes = m_soldier.Load(meshFileName);
	if (FAILED(hRes))
	{
		MessageBox(NULL, "Error", "Load Mesh Error", MB_OK);
		return E_FAIL;
	}
	delete meshFileName;

	//��ʼ���������
	m_deviceLost = false;

	return S_OK;		//��
}

//������ʲô��ʧ�ˣ�
void Application::OnDeviceLost()
{
	try
	{
		//��Щ������Ҫ��һ��OnLostDevice��
		global::pText->OnLostDevice();
		global::pSprite->OnLostDevice();
		global::pEffect->OnLostDevice();
		m_deviceLost = true;
	}
	catch (...)
	{
		global::streanOfDebug << "Error occured in Application::DeviceLost() \n";
	}
}

void Application::OnDeviceGained()
{
	try
	{
		global::pDevice->Reset(&m_present);		//ʲôʱ����Ҫ����һ�£�
		//��Щ������Ҫ��һ��OnResetDevice��
		global::pText->OnResetDevice();
		global::pSprite->OnResetDevice();
		global::pEffect->OnResetDevice();
		m_deviceLost = false;
	}
	catch (...)
	{
		global::streanOfDebug << "Error occured in Application::DeviceGained() \n";
	}
}

void Application::Update(float deltaTime)
{
	try
	{
		//�����ʾ�豸״̬
		HRESULT coop = global::pDevice->TestCooperativeLevel();		//j...
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

		//�Զ����߼�
		m_angle += deltaTime;

		//�������봦��
		if (KeyDown(VK_ESCAPE))
		{
			Quit();
		}

		if (KeyDown(VK_RETURN) && KeyDown(VK_MENU))		//ALT + RETURN
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
	}
	catch (...)		//... ��ʲô�﷨��
	{
		global::streanOfDebug << "Error in Application::Update() \n";
	}
}

void Application::Render()
{
	if (!m_deviceLost)
	{
		try
		{
			D3DXMATRIX identity, shadow;
			D3DXMatrixIdentity(&identity);


		}
		catch (...)
		{
			global::streanOfDebug << "Error in Application::Render() \n";
		}
	}
}

void Application::Quit()
{
	DestroyWindow(m_hwnd);	//�رմ���
	PostQuitMessage(0);		//��ֹ��Ϣѭ��
}