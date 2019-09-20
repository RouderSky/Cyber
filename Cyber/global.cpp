#include "global.h"

IDirect3DDevice9 *global::pDevice = NULL;

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
	if (pDevice != NULL)
		pDevice->Release();
	if (pText != NULL)
		pText->Release();
	if (pSprite != NULL)
		pSprite->Release();
	if (pEffect != NULL)
		pEffect->Release();

	streanOfDebug << "Application Terminated \n";

	if (streanOfDebug.good())
		streanOfDebug.close;
}

char* global::CombineStr(const char* str1, const char* str2)
{
	char *res = new char[strlen(str1) + strlen(str2)];

	strcpy(res, str1);
	strcat(res, str2);

	return res;
}