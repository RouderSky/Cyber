#ifndef _GLOBAL_
#define _GLOBAL_

#include <d3dx9.h>
#include <fstream>

#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600
#define ROOT_PATH_TO_TEXTURES "resources/Texture/"
#define ROOT_PATH_TO_EFFECT "resoutces/Effect/"
#define ROOT_PATH_TO_MESH "resoutces/Mesh/"

using namespace std;

namespace global
{
	//这是在创建全局变量吧？
	//这个头文件被多次include时,就算重定义了，全局变量不可以在头文件定义
	//感觉这些变量可以放进application里面
	IDirect3DDevice9 *pDevice = NULL;
	ID3DXSprite *pSprite = NULL;
	ID3DXFont *pText = NULL;
	ID3DXEffect *pEffect = NULL;
	ofstream streanOfDebug("debug.txt");
	/////////////////////////////////////////

	const D3DXCOLOR WHITE(D3DCOLOR_XRGB(255, 255, 255));
	const D3DXCOLOR BLACK(D3DCOLOR_XRGB(0, 0, 0));

	D3DMATERIAL9 InitMtrl(D3DXCOLOR a, D3DXCOLOR d, D3DXCOLOR s, D3DXCOLOR e, float p);
	const D3DMATERIAL9 mtrlOfWhite = InitMtrl(WHITE, WHITE, WHITE, BLACK, 1.0);

	void Release();
	char* CombineStr(const char *str1, const char *str2);
};

#endif