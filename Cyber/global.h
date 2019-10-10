#ifndef _GLOBAL_
#define _GLOBAL_

#include <d3dx9.h>
#include <fstream>

using namespace std;

#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600
#define NEAR_CLIP 0.1f
#define FAR_CLIP 100.0f
#define ROOT_PATH_TO_TEXTURES "resources/Texture/"
#define ROOT_PATH_TO_EFFECT "resources/Effect/"
#define ROOT_PATH_TO_MESH "resources/Mesh/"
#define ROOT_PATH_TO_MESH_TEXTURES "resources/Mesh/"

extern IDirect3DDevice9 *pD3DDevice;
extern ID3DXFont *pText;
extern ID3DXLine *pLine;

namespace global
{
	const D3DXCOLOR WHITE(D3DCOLOR_XRGB(255, 255, 255));
	const D3DXCOLOR BLACK(D3DCOLOR_XRGB(0, 0, 0));

	D3DMATERIAL9 InitMtrl(D3DXCOLOR a, D3DXCOLOR d, D3DXCOLOR s, D3DXCOLOR e, float p);
	const D3DMATERIAL9 mtrlOfWhite = InitMtrl(WHITE, WHITE, WHITE, BLACK, 1.0);

	void Release();
	char* CombineStr(const char *str1, const char *str2);

	bool KeyDown(int vk_code);
	bool KeyUp(int vk_code);

	string IntToString(int i);
	void ShowAllTrackStatus(ID3DXAnimationController* pAnimController);
};

#endif