#include "application.h"

IDirect3DDevice9 *pD3DDevice = NULL;
ID3DXSprite *pSprite = NULL;
ID3DXFont *pText = NULL;
ID3DXLine *pLine = NULL;
ofstream streanOfDebug("debug.txt");

float m_showTimeOfBandTxt = 0.0f;
class CallbackHandler : public ID3DXAnimationCallbackHandler
{
public:
	HRESULT CALLBACK HandleCallback(THIS_ UINT Track, LPVOID pCallbackData)
	{
		m_showTimeOfBandTxt = 0.25f;
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

	D3DXCreateLine(pD3DDevice, &pLine);

	//加载网格
	///无骨骼网格
	char *meshFileName = global::CombineStr(ROOT_PATH_TO_MESH, "soldier1.x");
	HRESULT hRes = m_soldier.Load(meshFileName, "Lighting.hlsl", "Shadow.hlsl");
	if (FAILED(hRes))
 		return E_FAIL;
	delete[]meshFileName;

	///有骨骼网格
	meshFileName = global::CombineStr(ROOT_PATH_TO_MESH, "soldier2.x");
	hRes = m_drone.Load(meshFileName, "Lighting.hlsl", "Shadow.hlsl");
	delete[]meshFileName;
	if (FAILED(hRes))
		return E_FAIL;

	////骨骼动画
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

	m_pOBB1 = new OBB(D3DXVECTOR3(2.0f, 1.0f, 0.3f));
	m_pOBB1->m_pos.x -= 1.5f;
	m_pOBB2 = new OBB(D3DXVECTOR3(0.3f, 1.0f, 2.0f));
	m_pOBB2->m_pos.x += 1.5f;
	D3DXQuaternionIdentity(&m_rot1);
	D3DXQuaternionIdentity(&m_rot2);
	m_cdOfPressSpace = 0.0f;

	//初始化其余变量
	m_deviceLost = false;

	return S_OK;		//？
}

int Application::EnterMsgLoop()
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
			Render(deltaTime);
			lastTime = curTime;
		}
	}

	return (int)msg.wParam;
}

//到底是什么丢失了？
void Application::OnDeviceLost()
{
	try
	{
		//哪些对象需要调一下OnLostDevice？
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
		pD3DDevice->Reset(&m_present);		//什么时候需要调用一下？
		//哪些对象需要调一下OnResetDevice？
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
		//m_angle += deltaTime;
		m_animation.Update(deltaTime);

		if (global::KeyDown(VK_RETURN))
		{
			Sleep(300);
			//RandomPlay4AnimFor4Model();
			RandomBlend2Animation();
		}

		if (m_showTimeOfBandTxt > 0.0f)
			m_showTimeOfBandTxt -= deltaTime;

		//OBB旋转
		if (global::KeyDown(VK_SPACE) && m_cdOfPressSpace <= 0.0f)
		{
			m_rot1 = D3DXQUATERNION(rand() % 1000 - 500.0f, rand() % 1000 - 500.0f, rand() % 1000 - 500.0f, rand() % 1000 - 500.0f);
			m_rot2 = D3DXQUATERNION(rand() % 1000 - 500.0f, rand() % 1000 - 500.0f, rand() % 1000 - 500.0f, rand() % 1000 - 500.0f);
			D3DXQuaternionNormalize(&m_rot1, &m_rot1);
			D3DXQuaternionNormalize(&m_rot2, &m_rot2);
			m_cdOfPressSpace = 0.3f;
		}
		m_cdOfPressSpace = max(m_cdOfPressSpace - deltaTime, 0.0f);

		//这样做可以实现从m_rot旋转到m_rot1的效果
		m_pOBB1->m_rot += m_rot1 * deltaTime;
		m_pOBB2->m_rot += m_rot2 * deltaTime;
		D3DXQuaternionNormalize(&m_pOBB1->m_rot, &m_pOBB1->m_rot);
		D3DXQuaternionNormalize(&m_pOBB2->m_rot, &m_pOBB2->m_rot);
	}
	catch (...)		//这是什么语法？
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
			D3DXVECTOR3 eyePos = eyeDir * 7.0f;
			D3DXMatrixLookAtLH(&view, &eyePos, &targetPos, &D3DXVECTOR3(0.0f, 1.0f, 0.0f));
			
			D3DXMATRIX proj;
			D3DXMatrixPerspectiveFovLH(&proj, D3DX_PI / 4.0f, (float)WINDOW_WIDTH / (float)WINDOW_HEIGHT, NEAR_CLIP, FAR_CLIP);

			//开始场景绘制
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

				/*
				m_animController->AdvanceTime(deltaTime * 0.5, &callbackHandler);
				m_drone.UpdateMatrixOfBone2Model();
				m_drone.HardRender(&view, &proj, &lightPos, &lightColor, &shadow);
				global::ShowAllTrackStatus(m_animController);

				if (m_showTimeOfBandTxt>0.0f)
				{
					RECT rc = { 0,0,WINDOW_WIDTH,WINDOW_HEIGHT };
					pText->DrawText(NULL, "BANG!", -1, &rc, DT_CENTER | DT_VCENTER | DT_NOCLIP, 0xFF000000);
					SetRect(&rc, -5, -5, WINDOW_WIDTH-5, WINDOW_HEIGHT-5);
					pText->DrawText(NULL, "BANG!", -1, &rc, DT_CENTER | DT_VCENTER | DT_NOCLIP, 0xFFFFFF00);
				}
				*/

				//m_animation.Draw();

				m_pOBB1->Render(&view, &proj, &lightPos, &lightColor, &shadow);
				m_pOBB2->Render(&view, &proj, &lightPos, &lightColor, &shadow);
				if (m_pOBB1->Intersect(*m_pOBB2))
				{
					RECT rc = { 10, 10, 0, 0 };
					pText->DrawText(NULL, "Intersecting!", -1, &rc, DT_LEFT | DT_TOP | DT_NOCLIP, 0xff000000);
				}

				pD3DDevice->EndScene();
				pD3DDevice->Present(0, 0, 0, 0);
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

	//随机抽取两个动画，并放到两个轨道上
	int numAnimations = m_animController->GetMaxNumAnimationSets();
	ID3DXAnimationSet* anim1 = NULL;
	ID3DXAnimationSet* anim2 = NULL;
	m_animController->GetAnimationSet(rand() % numAnimations, &anim1);
	m_animController->GetAnimationSet(rand() % numAnimations, &anim2);
	m_animController->SetTrackAnimationSet(0, anim1);
	m_animController->SetTrackAnimationSet(1, anim2);

	//混合
	float w = (rand() % 1000) / 1000.0f;
	m_animController->SetTrackWeight(0, w);
	m_animController->SetTrackWeight(1, 1.0f - w);
	m_animController->SetTrackSpeed(0, (rand() % 1000) / 500.0f);
	m_animController->SetTrackSpeed(1, (rand() % 1000) / 500.0f);
	m_animController->SetTrackPriority(0, D3DXPRIORITY_HIGH);		//todo：改掉优先级试试
	m_animController->SetTrackPriority(1, D3DXPRIORITY_HIGH);
	m_animController->SetTrackEnable(0, true);
	m_animController->SetTrackEnable(1, true);
}

void Application::TestCallback()
{
	//随便拿一个动画
	ID3DXKeyframedAnimationSet* animSet = NULL;
	m_animController->GetAnimationSet(1, (ID3DXAnimationSet**)&animSet);

	//压缩动画
	ID3DXBuffer* compressedData = NULL;
	animSet->Compress(D3DXCOMPRESS_DEFAULT, 0.5f, NULL, &compressedData);

	//创建callback
	const UINT numCallbacks = 1;
	D3DXKEY_CALLBACK keys[numCallbacks];
	double ticksPerSecond = animSet->GetSourceTicksPerSecond();
	keys[0].Time = float(animSet->GetPeriod() / 2.0f*ticksPerSecond);
	keys[0].pCallbackData = (void*)&m_drone;
	ID3DXCompressedAnimationSet* compressedAnimSet = NULL;
	//添加callback一定要压缩动画
	D3DXCreateCompressedAnimationSet(
		animSet->GetName(),
		animSet->GetSourceTicksPerSecond(),
		animSet->GetPlaybackType(),
		compressedData,
		numCallbacks,
		keys,
		&compressedAnimSet);
	compressedData->Release();

	m_animController->UnregisterAnimationSet(animSet);			//可不可以早点删除？
	m_animController->RegisterAnimationSet(compressedAnimSet);
	m_animController->SetTrackAnimationSet(0, compressedAnimSet);
	compressedAnimSet->Release();
}

void Application::Quit()
{
	DestroyWindow(m_hwnd);	//关闭窗口
	PostQuitMessage(0);		//终止消息循环
}