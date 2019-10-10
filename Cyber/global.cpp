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

char* global::CombineStr(const char* str1, const char* str2)
{
	char *res = new char[strlen(str1) + strlen(str2) + 1];

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

string global::IntToString(int i)
{
	char num[10];
	_itoa(i, num, 10);
	return num;
}

void global::ShowAllTrackStatus(ID3DXAnimationController* pAnimController)
{
	pLine->SetWidth(100.0f);
	pLine->Begin();
	D3DXVECTOR2 p[] = { D3DXVECTOR2(0, 550), D3DXVECTOR2(800, 550) };
	pLine->Draw(p, 2, 0x88FFFFFF);
	pLine->End();

	int numTracks = pAnimController->GetMaxNumTracks();
	for (int i = 0; i < numTracks; i++)
	{
		D3DXTRACK_DESC desc;
		ID3DXAnimationSet* anim = NULL;
		pAnimController->GetTrackDesc(i, &desc);
		pAnimController->GetTrackAnimationSet(i, &anim);

		if (anim != NULL)
		{
			string animName = anim->GetName();
			while (animName.size() < 10)
				animName.push_back(' ');

			string s = string("Track #") + global::IntToString(i + 1) + animName;
			s += string("Weight = ") + global::IntToString((int)(desc.Weight * 100)) + "%";
			s += string(", Position = ") + global::IntToString((int)(desc.Position * 1000)) + " ms";	//µ¥Î»ÊÇÃë£¿
			s += string(", Speed = ") + global::IntToString((int)(desc.Speed * 100)) + "%";

			RECT r = { 10, 530 + i * 20, 0, 0 };
			pText->DrawText(NULL, s.c_str(), -1, &r, DT_LEFT | DT_TOP | DT_NOCLIP, 0xAA000000);
		}
	}
}