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
	//�����ڴ���ȫ�ֱ����ɣ�
	//���ͷ�ļ������includeʱ,�����ض����ˣ�ȫ�ֱ�����������ͷ�ļ�����
	//�о���Щ�������ԷŽ�application����
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