#include "OBB.h"
#include "global.h"

extern IDirect3DDevice9* pD3DDevice;

OBB::OBB()
{
	m_pMesh = NULL;
	m_pos = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
	D3DXQuaternionIdentity(&m_rot);
	SetSize(D3DXVECTOR3(1.0f, 1.0f, 1.0f));

	//加载shader文件
	ID3DXBuffer *pErrorMsgs = NULL;
	char *effectFileName = global::CombineStr(ROOT_PATH_TO_EFFECT, "Lighting.hlsl");
	HRESULT hRes = D3DXCreateEffectFromFile(
		pD3DDevice,
		effectFileName,
		NULL,
		NULL,
		D3DXSHADER_DEBUG,
		NULL,
		&pLightingEffect,
		&pErrorMsgs);
	delete[]effectFileName;
	if (FAILED(hRes) && (pErrorMsgs != NULL))
		MessageBox(NULL, (char*)pErrorMsgs->GetBufferPointer(), "Load Lighting Effect Error", MB_OK);		//MB_OK是啥？
}

OBB::OBB(D3DXVECTOR3 halfSize)
{
	m_pMesh = NULL;
	m_pos = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
	D3DXQuaternionIdentity(&m_rot);
	SetSize(halfSize);

	//加载shader文件
	ID3DXBuffer *pErrorMsgs = NULL;
	char *effectFileName = global::CombineStr(ROOT_PATH_TO_EFFECT, "Lighting.hlsl");
	HRESULT hRes = D3DXCreateEffectFromFile(
		pD3DDevice,
		effectFileName,
		NULL,
		NULL,
		D3DXSHADER_DEBUG,
		NULL,
		&pLightingEffect,
		&pErrorMsgs);
	delete[]effectFileName;
	if (FAILED(hRes) && (pErrorMsgs != NULL))
		MessageBox(NULL, (char*)pErrorMsgs->GetBufferPointer(), "Load Lighting Effect Error", MB_OK);
}

OBB::~OBB()
{
	Release();
}

void OBB::Release()
{
	if (m_pMesh != NULL)
	{
		m_pMesh->Release();
		m_pMesh = NULL;
	}
}

void OBB::SetSize(D3DXVECTOR3 halfSize)
{
	m_halfSize = halfSize;
	Release();
	D3DXCreateBox(pD3DDevice, m_halfSize.x * 2.0f, m_halfSize.y * 2.0f, m_halfSize.z * 2.0f, &m_pMesh, NULL);
	m_HalfDiagonalLength = D3DXVec3Length(&m_halfSize);
}

void OBB::Render(D3DXMATRIX *view, D3DXMATRIX *proj, D3DXVECTOR4 *lightPos, D3DXVECTOR4 *lightColor, D3DXMATRIX *shadow)
{
	if (m_pMesh != NULL)
	{
		D3DXMATRIX matSelfPos, matSelfRot, matSelf2World;
		D3DXMatrixTranslation(&matSelfPos, m_pos.x, m_pos.y, m_pos.z);
		D3DXMatrixRotationQuaternion(&matSelfRot, &m_rot);
		D3DXMatrixMultiply(&matSelf2World, &matSelfPos, &matSelfPos);

		pLightingEffect->SetMatrix("matW", &matSelf2World);
		pLightingEffect->SetMatrix("matVP", &((*view) * (*proj)));
		pLightingEffect->SetVector("lightPos", lightPos);
		D3DXHANDLE hTech = pLightingEffect->GetTechniqueByName("NormalLighting");
		pLightingEffect->SetTechnique(hTech);
		UINT passCount = 0;
		pLightingEffect->Begin(&passCount, NULL);
		for (UINT i = 0; i < passCount; ++i)
		{
			pLightingEffect->BeginPass(i);
			m_pMesh->DrawSubset(0);
			pLightingEffect->EndPass();
		}
		pLightingEffect->End();
	}
}

bool OBB::Intersect(OBB &b)
{
	float distanceBetween2Box = D3DXVec3Length(&(b.m_pos - m_pos));
	if (distanceBetween2Box > (b.m_HalfDiagonalLength + m_HalfDiagonalLength))
		return false;

	D3DXMATRIX matSelf2World, matB2World, matB2Self;
	D3DXMatrixRotationQuaternion(&matSelf2World, &m_rot);
	D3DXMatrixRotationQuaternion(&matB2World, &b.m_rot);

	D3DXMATRIX matWorld2Self;
	D3DXMatrixInverse(&matWorld2Self, NULL, &matSelf2World);
	D3DXMatrixMultiply(&matB2Self, &matB2World, &matWorld2Self);

	D3DXVECTOR4 bPosInSelf;
	D3DXVec3Transform(&bPosInSelf, &(b.m_pos - m_pos), &matWorld2Self);

	D3DXVECTOR3 bXAxisInSelf(matB2Self._11, matB2Self._21, matB2Self._31);
	D3DXVECTOR3 bYAxisOfBInSelf(matB2Self._12, matB2Self._22, matB2Self._32);
	D3DXVECTOR3 bZAxisOfBInSelf(matB2Self._13, matB2Self._23, matB2Self._33);

	//下面完全不知道在干嘛...
	//15 tests
	//1 (Ra)x
	if (fabs(bPosInSelf.x) > m_halfSize.x + b.m_halfSize.x * fabs(bXAxisInSelf.x) + b.m_halfSize.y * fabs(bXAxisInSelf.y) + b.m_halfSize.z * fabs(bXAxisInSelf.z))
		return false;
	//2 (Ra)y
	if (fabs(bPosInSelf.y) > m_halfSize.y + b.m_halfSize.x * fabs(bYAxisOfBInSelf.x) + b.m_halfSize.y * fabs(bYAxisOfBInSelf.y) + b.m_halfSize.z * fabs(bYAxisOfBInSelf.z))
		return false;
	//3 (Ra)z
	if (fabs(bPosInSelf.z) > m_halfSize.z + b.m_halfSize.x * fabs(bZAxisOfBInSelf.x) + b.m_halfSize.y * fabs(bZAxisOfBInSelf.y) + b.m_halfSize.z * fabs(bZAxisOfBInSelf.z))
		return false;

	//4 (Rb)x
	if (fabs(bPosInSelf.x*bXAxisInSelf.x + bPosInSelf.y*bYAxisOfBInSelf.x + bPosInSelf.z*bZAxisOfBInSelf.x) > (b.m_halfSize.x + m_halfSize.x*fabs(bXAxisInSelf.x) + m_halfSize.y * fabs(bYAxisOfBInSelf.x) + m_halfSize.z*fabs(bZAxisOfBInSelf.x)))
		return false;
	//5 (Rb)y
	if (fabs(bPosInSelf.x*bXAxisInSelf.y + bPosInSelf.y*bYAxisOfBInSelf.y + bPosInSelf.z*bZAxisOfBInSelf.y) > (b.m_halfSize.y + m_halfSize.x*fabs(bXAxisInSelf.y) + m_halfSize.y * fabs(bYAxisOfBInSelf.y) + m_halfSize.z*fabs(bZAxisOfBInSelf.y)))
		return false;
	//6 (Rb)z
	if (fabs(bPosInSelf.x*bXAxisInSelf.z + bPosInSelf.y*bYAxisOfBInSelf.z + bPosInSelf.z*bZAxisOfBInSelf.z) > (b.m_halfSize.z + m_halfSize.x*fabs(bXAxisInSelf.z) + m_halfSize.y * fabs(bYAxisOfBInSelf.z) + m_halfSize.z*fabs(bZAxisOfBInSelf.z)))
		return false;

	//7 (Ra)x X (Rb)x
	if (fabs(bPosInSelf.z*bYAxisOfBInSelf.x - bPosInSelf.y*bZAxisOfBInSelf.x) > m_halfSize.y*fabs(bZAxisOfBInSelf.x) + m_halfSize.z*fabs(bYAxisOfBInSelf.x) + b.m_halfSize.y*fabs(bXAxisInSelf.z) + b.m_halfSize.z*fabs(bXAxisInSelf.y))
		return false;
	//8 (Ra)x X (Rb)y
	if (fabs(bPosInSelf.z*bYAxisOfBInSelf.y - bPosInSelf.y*bZAxisOfBInSelf.y) > m_halfSize.y*fabs(bZAxisOfBInSelf.y) + m_halfSize.z*fabs(bYAxisOfBInSelf.y) + b.m_halfSize.x*fabs(bXAxisInSelf.z) + b.m_halfSize.z*fabs(bXAxisInSelf.x))
		return false;
	//9 (Ra)x X (Rb)z
	if (fabs(bPosInSelf.z*bYAxisOfBInSelf.z - bPosInSelf.y*bZAxisOfBInSelf.z) > m_halfSize.y*fabs(bZAxisOfBInSelf.z) + m_halfSize.z*fabs(bYAxisOfBInSelf.z) + b.m_halfSize.x*fabs(bXAxisInSelf.y) + b.m_halfSize.y*fabs(bXAxisInSelf.x))
		return false;

	//10 (Ra)y X (Rb)x
	if (fabs(bPosInSelf.x*bZAxisOfBInSelf.x - bPosInSelf.z*bXAxisInSelf.x) > m_halfSize.x*fabs(bZAxisOfBInSelf.x) + m_halfSize.z*fabs(bXAxisInSelf.x) + b.m_halfSize.y*fabs(bYAxisOfBInSelf.z) + b.m_halfSize.z*fabs(bYAxisOfBInSelf.y))
		return false;
	//11 (Ra)y X (Rb)y
	if (fabs(bPosInSelf.x*bZAxisOfBInSelf.y - bPosInSelf.z*bXAxisInSelf.y) > m_halfSize.x*fabs(bZAxisOfBInSelf.y) + m_halfSize.z*fabs(bXAxisInSelf.y) + b.m_halfSize.x*fabs(bYAxisOfBInSelf.z) + b.m_halfSize.z*fabs(bYAxisOfBInSelf.x))
		return false;
	//12 (Ra)y X (Rb)z
	if (fabs(bPosInSelf.x*bZAxisOfBInSelf.z - bPosInSelf.z*bXAxisInSelf.z) > m_halfSize.x*fabs(bZAxisOfBInSelf.z) + m_halfSize.z*fabs(bXAxisInSelf.z) + b.m_halfSize.x*fabs(bYAxisOfBInSelf.y) + b.m_halfSize.y*fabs(bYAxisOfBInSelf.x))
		return false;

	//13 (Ra)z X (Rb)x
	if (fabs(bPosInSelf.y*bXAxisInSelf.x - bPosInSelf.x*bYAxisOfBInSelf.x) > m_halfSize.x*fabs(bYAxisOfBInSelf.x) + m_halfSize.y*fabs(bXAxisInSelf.x) + b.m_halfSize.y*fabs(bZAxisOfBInSelf.z) + b.m_halfSize.z*fabs(bZAxisOfBInSelf.y))
		return false;
	//14 (Ra)z X (Rb)y
	if (fabs(bPosInSelf.y*bXAxisInSelf.y - bPosInSelf.x*bYAxisOfBInSelf.y) > m_halfSize.x*fabs(bYAxisOfBInSelf.y) + m_halfSize.y*fabs(bXAxisInSelf.y) + b.m_halfSize.x*fabs(bZAxisOfBInSelf.z) + b.m_halfSize.z*fabs(bZAxisOfBInSelf.x))
		return false;
	//15 (Ra)z X (Rb)z
	if (fabs(bPosInSelf.y*bXAxisInSelf.z - bPosInSelf.x*bYAxisOfBInSelf.z) > m_halfSize.x*fabs(bYAxisOfBInSelf.z) + m_halfSize.y*fabs(bXAxisInSelf.z) + b.m_halfSize.x*fabs(bZAxisOfBInSelf.y) + b.m_halfSize.y*fabs(bZAxisOfBInSelf.x))
		return false;

	return true;
}

bool OBB::Intersect(Ray & ray)
{
	D3DXMATRIX matSelfPos, matSelfRot, matSelf2World, matWorld2Self;
	D3DXMatrixTranslation(&matSelfPos, m_pos.x, m_pos.y, m_pos.z);
	D3DXMatrixRotationQuaternion(&matSelfRot, &m_rot);
	D3DXMatrixMultiply(&matSelf2World, &matSelfRot, &matSelfPos);
	D3DXMatrixInverse(&matWorld2Self, NULL, &matSelf2World);

	D3DXVECTOR3 org, dir;
	D3DXVec3TransformCoord(&org, &ray.m_org, &matWorld2Self);
	D3DXVec3TransformNormal(&dir, &ray.m_dir, &matWorld2Self);

	return D3DXBoxBoundProbe(&(-m_halfSize), &m_halfSize, &org, &dir);
}

bool OBB::Intersect(D3DXVECTOR3 & point)
{
	D3DXMATRIX matSelfPos, matSelfRot, matSelf2World, matWorld2Self;
	D3DXMatrixTranslation(&matSelfPos, m_pos.x, m_pos.y, m_pos.z);
	D3DXMatrixRotationQuaternion(&matSelfRot, &m_rot);
	D3DXMatrixMultiply(&matSelf2World, &matSelfRot, &matSelfPos);
	D3DXMatrixInverse(&matWorld2Self, NULL, &matSelf2World);

	D3DXVECTOR4 posInSelf;

	if (abs(posInSelf.x) > m_halfSize.x) return false;
	if (abs(posInSelf.y) > m_halfSize.y) return false;
	if (abs(posInSelf.z) > m_halfSize.z) return false;

	return true;
}

D3DXVECTOR3 OBB::GetContactPointWithRay(Ray & ray)
{
	D3DXMATRIX matSelfPos, matSelfRot, matSelf2World, matWorld2Self;
	D3DXMatrixTranslation(&matSelfPos, m_pos.x, m_pos.y, m_pos.z);
	D3DXMatrixRotationQuaternion(&matSelfRot, &m_rot);
	D3DXMatrixMultiply(&matSelf2World, &matSelfRot, &matSelfPos);
	D3DXMatrixInverse(&matWorld2Self, NULL, &matSelf2World);

	D3DXVECTOR3 rayOrgInSelf, rayDirInSelf;
	D3DXVec3TransformCoord(&rayOrgInSelf, &ray.m_org, &matWorld2Self);
	D3DXVec3TransformNormal(&rayDirInSelf, &ray.m_dir, &matWorld2Self);

	D3DXPLANE planes[] = {
		D3DXPLANE(0.0f, 0.0f, -1.0f, -m_halfSize.z),
		D3DXPLANE(0.0f, 0.0f, 1.0f, -m_halfSize.z),
		D3DXPLANE(0.0f, -1.0f, 0.0f, -m_halfSize.y),
		D3DXPLANE(0.0f, 1.0f, 0.0f, -m_halfSize.y),
		D3DXPLANE(-1.0f, 0.0f, 0.0f, -m_halfSize.x),
		D3DXPLANE(1.0f, 0.0f, 0.0f, -m_halfSize.x) };

	D3DXVECTOR3 result;
	int numOfRayIntersectPanel = 0;
	int realNumOfRayIntersectPanel = 0;

	for (int i=0;i<6;++i)
	{
		float d = rayOrgInSelf.x * planes[i].a +
			rayOrgInSelf.y * planes[i].b +
			rayOrgInSelf.z * planes[i].c;

		if (d > -planes[i].d)	//射线原点在平面正面
		{
			D3DXVECTOR3 intersectPos;
			if (D3DXPlaneIntersectLine(&intersectPos, &planes[i], &rayOrgInSelf, &(rayOrgInSelf + rayDirInSelf * 1000.0f))!=NULL)		//射线和平面相交
			{
				numOfRayIntersectPanel++;
				if (abs(intersectPos.x) <= m_halfSize.x + 0.01f &&
					abs(intersectPos.y) <= m_halfSize.y + 0.01f &&
					abs(intersectPos.z) <= m_halfSize.z + 0.01f)
				{
					D3DXVec3TransformCoord(&result, &intersectPos, &matSelf2World);
					realNumOfRayIntersectPanel++;
				}
			}
		}
	}

	return result;
}

vector<D3DXVECTOR3> OBB::GetAllCornersOfBox()
{
	D3DXMATRIX matSelfPos, matSelfRot, matSelf2World;
	D3DXMatrixTranslation(&matSelfPos, m_pos.x, m_pos.y, m_pos.z);
	D3DXMatrixRotationQuaternion(&matSelfRot, &m_rot);
	D3DXMatrixMultiply(&matSelf2World, &matSelfRot, &matSelfPos);

	vector<D3DXVECTOR3> allCorners;
	allCorners.push_back(D3DXVECTOR3(m_halfSize.x, m_halfSize.y, m_halfSize.z));
	allCorners.push_back(D3DXVECTOR3(-m_halfSize.x, m_halfSize.y, m_halfSize.z));
	allCorners.push_back(D3DXVECTOR3(m_halfSize.x, -m_halfSize.y, m_halfSize.z));
	allCorners.push_back(D3DXVECTOR3(m_halfSize.x, m_halfSize.y, -m_halfSize.z));
	allCorners.push_back(D3DXVECTOR3(m_halfSize.x, -m_halfSize.y, -m_halfSize.z));
	allCorners.push_back(D3DXVECTOR3(-m_halfSize.x, m_halfSize.y, -m_halfSize.z));
	allCorners.push_back(D3DXVECTOR3(-m_halfSize.x, -m_halfSize.y, m_halfSize.z));
	allCorners.push_back(D3DXVECTOR3(-m_halfSize.x, -m_halfSize.y, -m_halfSize.z));

	int numCorners = (int)allCorners.size();
	for (int i=0;i<numCorners;++i)
	{
		D3DXVECTOR4 v4;
		D3DXVec3Transform(&v4, &allCorners[i], &matSelf2World);
		allCorners[i] = D3DXVECTOR3(v4.x, v4.y, v4.z);
	}

	return allCorners;
}
