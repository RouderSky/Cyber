#include "application.h"

Application::Application()
{
	m_angle = 0.0f;
}

Application::~Application()
{
	global::Release();
}

//CALLBACK什么意思？
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

	//注册一个窗口类
	WNDCLASS wc;
	memset(&wc, 0, sizeof(WNDCLASS));
	wc.style = CS_HREDRAW | CS_VREDRAW;		//CS_XXX代表什么？
	wc.lpfnWndProc = WndProc;
	wc.hInstance = hAppIns;
	wc.lpszClassName = "D3DWND";
	RECT rc = { 0, 0, WINDOW_WIDTH, WINDOW_HEIGHT };
	AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, false);	//？
	RegisterClass(&wc);
	
	//创建一个窗口实例
	//屏幕空间原点在左上角
	m_hwnd = CreateWindow(
		"D3DWND",
		"Cyber",
		WS_OVERLAPPEDWINDOW,	//？
		0, 0,
		rc.right - rc.left,
		rc.bottom - rc.top,
		NULL,
		NULL,
		hAppIns,
		0);

	//设置鼠标
	//SetCursor(NULL);	//隐藏鼠标

	//显示窗口
	ShowWindow(m_hwnd, SW_SHOW);
	UpdateWindow(m_hwnd);		//什么情况需要调一下？

	//创建d3d管理器
	IDirect3D9* d3d9 = Direct3DCreate9(D3D_SDK_VERSION);
	if (d3d9 == NULL)
	{
		global::streanOfDebug << "Direct3DCreate9() - FAILED \n";
		return E_FAIL;
	}

	//设备性能检测
	D3DCAPS9 caps;
	d3d9->GetDeviceCaps(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, &caps);
	if (caps.VertexShaderVersion < D3DVS_VERSION(2, 0) || caps.PixelShaderVersion < D3DPS_VERSION(2, 0))
		global::streanOfDebug << "Warning - Your graphic card does not support vertec and pixelshaders version 2.0 \n";

	int typeOfSupportedTransAndLight = 0;
	if (caps.DevCaps & D3DDEVCAPS_HWTRANSFORMANDLIGHT)
		typeOfSupportedTransAndLight = D3DCREATE_HARDWARE_VERTEXPROCESSING;
	else
		typeOfSupportedTransAndLight = D3DCREATE_SOFTWARE_VERTEXPROCESSING;

	//创建显示设备
	m_present.BackBufferWidth = WINDOW_WIDTH;
	m_present.BackBufferHeight = WINDOW_HEIGHT;
	m_present.BackBufferFormat = D3DFMT_A8R8G8B8;
	m_present.BackBufferCount = 2;
	m_present.MultiSampleType = D3DMULTISAMPLE_NONE;	//应该是抗锯齿
	m_present.MultiSampleQuality = 0;					//抗锯齿质量？
	m_present.SwapEffect = D3DSWAPEFFECT_DISCARD;		//？
	m_present.hDeviceWindow = m_hwnd;
	m_present.Windowed = windowed;
	m_present.EnableAutoDepthStencil = true;			//？
	m_present.AutoDepthStencilFormat = D3DFMT_D24S8;
	m_present.Flags = 0;								//？
	m_present.FullScreen_RefreshRateInHz = D3DPRESENT_RATE_DEFAULT;		//刷新率？
	m_present.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;		//？
	if (FAILED(d3d9->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, m_hwnd, typeOfSupportedTransAndLight, &m_present, &global::pDevice)))
	{
		global::streanOfDebug << "Failed to create IDirect3DDevice9 \n";
		return E_FAIL;
	}

	//释放d3d管理器
	d3d9->Release();

	//创建显示文本
	D3DXCreateFont(global::pDevice, 20, 0, FW_BOLD, 1, false,
		DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, DEFAULT_QUALITY,
		DEFAULT_PITCH | FF_DONTCARE, "Arial", &global::pText);

	//？
	D3DXCreateSprite(global::pDevice, &global::pSprite);

	//加载shader文件
	ID3DXBuffer *pErrorMsgs = NULL;
	char *effectFileName = global::CombineStr(ROOT_PATH_TO_EFFECT, "lightting.fx");
	HRESULT hRes = D3DXCreateEffectFromFile(global::pDevice, effectFileName);
	delete effectFileName;
	if (FAILED(hRes) && (pErrorMsgs != NULL))
	{
		MessageBox(NULL, (char*)pErrorMsgs->GetBufferPointer(), "Load Effect Error", MB_OK);		//MB_OK是啥？
		return E_FAIL;		//？
	}

	//加载网格
	char *meshFileName = global::CombineStr(ROOT_PATH_TO_MESH, "soldier.x");
	hRes = m_soldier.Load(meshFileName);
	if (FAILED(hRes))
	{
		MessageBox(NULL, "Error", "Load Mesh Error", MB_OK);
		return E_FAIL;
	}
	delete meshFileName;

	//初始化其余变量
	m_deviceLost = false;

	return S_OK;		//？
}

//到底是什么丢失了？
void Application::OnDeviceLost()
{
	try
	{
		//哪些对象需要调一下OnLostDevice？
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
		global::pDevice->Reset(&m_present);		//什么时候需要调用一下？
		//哪些对象需要调一下OnResetDevice？
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
		//检查显示设备状态
		HRESULT coop = global::pDevice->TestCooperativeLevel();		//j...
		if (coop != D3D_OK)
		{
			if (coop == D3DERR_DEVICELOST)		//到底是什么东西丢失了？
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

		//自定义逻辑
		m_angle += deltaTime;

		//键盘输入处理
		if (KeyDown(VK_ESCAPE))
		{
			Quit();
		}

		if (KeyDown(VK_RETURN) && KeyDown(VK_MENU))		//ALT + RETURN
		{
			//切换功能好像不行Release
			m_present.Windowed = !m_present.Windowed;

			//为何要手动调用，没法通过TestCooperativeLevel检测到？
			OnDeviceLost();
			OnDeviceGained();

			if (m_present.Windowed)
			{
				RECT rc = { 0, 0, WINDOW_WIDTH, WINDOW_HEIGHT };
				AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, false);
				SetWindowPos(m_hwnd, HWND_NOTOPMOST, 0, 0, rc.right - rc.left, rc.bottom - rc.top, SWP_SHOWWINDOW);		//和AdjustWindowRect有什么区别？
				UpdateWindow(m_hwnd);
			}
		}
	}
	catch (...)		//... 是什么语法？
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
	DestroyWindow(m_hwnd);	//关闭窗口
	PostQuitMessage(0);		//终止消息循环
}