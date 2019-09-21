#include "global.h"

D3DMATERIAL9 global::InitMtrl(D3DXCOLOR a, D3DXCOLOR d, D3DXCOLOR s, D3DXCOLOR e, float p)
{
	D3DMATERIAL9 mtrl;
	mtrl.Ambient = a;
	mtrl.Diffuse = d;
	mtrl.Specular = s;
	mtrl.Emissive = e;
	mtrl.Power = p;
	return mtrl;
}

void global::Release()
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

char* global::CombineStr(const char* str1, const char* str2)
{
	char *res = new char[strlen(str1) + strlen(str2)];

	strcpy(res, str1);
	strcat(res, str2);

	return res;
}

bool global::KeyDown(int vk_code)
{
	return (GetAsyncKeyState(vk_code) & 0x8000) ? true : false;		//£¿
}

bool global::KeyUp(int vk_code)
{
	return !KeyDown(vk_code);
}