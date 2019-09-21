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
	streanOfDebug << "Applocation Started \n";

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
		streanOfDebug << "Direct3DCreate9() - FAILED \n";
		return E_FAIL;		//？
	}

	//设备性能检测
	D3DCAPS9 caps;
	d3d9->GetDeviceCaps(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, &caps);
	if (caps.VertexShaderVersion < D3DVS_VERSION(2, 0) || caps.PixelShaderVersion < D3DPS_VERSION(2, 0))
		streanOfDebug << "Warning - Your graphic card does not support vertec and pixelshaders version 2.0 \n";

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
	if (FAILED(d3d9->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, m_hwnd, typeOfSupportedTransAndLight, &m_present, &pD3DDevice)))
	{
		streanOfDebug << "Failed to create IDirect3DDevice9 \n";
		return E_FAIL;
	}

	//释放d3d管理器
	d3d9->Release();

	//创建显示文本
	D3DXCreateFont(pD3DDevice, 20, 0, FW_BOLD, 1, false,
		DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, DEFAULT_QUALITY,
		DEFAULT_PITCH | FF_DONTCARE, "Arial", &pText);

	//？
	D3DXCreateSprite(pD3DDevice, &pSprite);

	//加载shader文件
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
	//delete effectFileName;		//为啥delete会报错？
	if (FAILED(hRes) && (pErrorMsgs != NULL))
	{
		MessageBox(NULL, (char*)pErrorMsgs->GetBufferPointer(), "Load LambertDiffuse Effect Error", MB_OK);		//MB_OK是啥？
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
		MessageBox(NULL, (char*)pErrorMsgs->GetBufferPointer(), "Load Shadow Effect Error", MB_OK);		//MB_OK是啥？
		return E_FAIL;
	}

	//加载网格
	char *meshFileName = global::CombineStr(ROOT_PATH_TO_MESH, "soldier.x");
	hRes = m_soldier.Load(meshFileName);
	if (FAILED(hRes))
	{
		MessageBox(NULL, "Error", "Load Mesh Error", MB_OK);
		return E_FAIL;
	}
	//delete meshFileName;

	//初始化其余变量
	m_deviceLost = false;

	return S_OK;		//？
}

void Application::EnterMsgLoop()
{
	DWORD lastTime = GetTickCount();
	MSG msg;
	memset(&msg, 0, sizeof(MSG));
	while (msg.message != WM_QUIT)		//当收到退出消息循环的消息时退出
	{
		if (PeekMessage(&msg, 0, 0, 0, PM_REMOVE))		//这些参数是什么意思？
		{
			//有系统消息就处理
			TranslateMessage(&msg);	//？
			DispatchMessage(&msg);	//？
		}
		else
		{
			//没有系统消息就干自己的事情
			DWORD curTime = GetTickCount();
			float deltaTime = (curTime - lastTime)*0.001f;
			Update(deltaTime);
			Render();
			lastTime = curTime;
		}
	}
}

//到底是什么丢失了？
void Application::OnDeviceLost()
{
	try
	{
		//哪些对象需要调一下OnLostDevice？
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
		pD3DDevice->Reset(&m_present);		//什么时候需要调用一下？
		//哪些对象需要调一下OnResetDevice？
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
		//检查显示设备状态
		HRESULT coop = pD3DDevice->TestCooperativeLevel();		//j...
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

		//键盘输入处理
		if (global::KeyDown(VK_ESCAPE))
		{
			Quit();
		}

		if (global::KeyDown(VK_RETURN) && global::KeyDown(VK_MENU))		//ALT + RETURN
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

		//自定义逻辑
		m_angle += deltaTime;
	}
	catch (...)		//... 是什么语法？
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
			D3DXVECTOR3 targetPos(0.0f, 1.0f, 0.0f);		//应该根据world的位置来计算才对...
			D3DXVECTOR3 eyeDir(cos(m_angle), 1.0f, sin(m_angle));
			float distanceFromEye2Tar = 2.0;
			D3DXVECTOR3 eyePos = targetPos + eyeDir * distanceFromEye2Tar;
			D3DXMatrixLookAtRH(&view, &eyePos, &targetPos, &D3DXVECTOR3(0.0f, 1.0f, 0.0f));
			
			D3DXMATRIX proj;
			D3DXMatrixPerspectiveFovLH(&proj, D3DX_PI / 4.0f, (float)WINDOW_WIDTH / (float)WINDOW_HEIGHT, NEAR_CLIP, FAR_CLIP);
		
			//这个不用设置也可以吧？
			pD3DDevice->SetTransform(D3DTS_WORLD, &world);
			pD3DDevice->SetTransform(D3DTS_VIEW, &view);
			pD3DDevice->SetTransform(D3DTS_PROJECTION, &proj);
			//////////////////////////////////////////////////////////

			//开始场景绘制
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
	DestroyWindow(m_hwnd);	//关闭窗口
	PostQuitMessage(0);		//终止消息循环
}