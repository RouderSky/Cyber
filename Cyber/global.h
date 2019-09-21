#ifndef _GLOBAL_
#define _GLOBAL_

#pragma warning(disable:4996)

#include <d3dx9.h>
#include <fstream>

using namespace std;

#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600
#define NEAR_CLIP 0.1f
#define FAR_CLIP 100.0f
#define ROOT_PATH_TO_TEXTURES "resources/Mesh/"
#define ROOT_PATH_TO_EFFECT "resources/Effect/"
#define ROOT_PATH_TO_MESH "resources/Mesh/"

namespace global
{
	//头文件被多次include时，全局变量就算重定义了，所以全局变量不可以在头文件定义；
	//但静态变量被多次include时不算重定义，所以可以放在头文件中
// 	static IDirect3DDevice9 *pD3DDevice = NULL;
// 	static ID3DXSprite *pSprite = NULL;
// 	static ID3DXFont *pText = NULL;
// 	static ID3DXEffect *pLambertDiffuseEffect = NULL;
// 	static ID3DXEffect *pShadowEffect = NULL;
// 	static ofstream streanOfDebug("debug.txt");

	const D3DXCOLOR WHITE(D3DCOLOR_XRGB(255, 255, 255));
	const D3DXCOLOR BLACK(D3DCOLOR_XRGB(0, 0, 0));

	D3DMATERIAL9 InitMtrl(D3DXCOLOR a, D3DXCOLOR d, D3DXCOLOR s, D3DXCOLOR e, float p);
	const D3DMATERIAL9 mtrlOfWhite = InitMtrl(WHITE, WHITE, WHITE, BLACK, 1.0);

	void Release();
	char* CombineStr(const char *str1, const char *str2);

	bool KeyDown(int vk_code);
	bool KeyUp(int vk_code);
};

#endif