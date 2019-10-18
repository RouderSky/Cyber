#ifndef OBB_H
#define OBB_H

#include <d3dx9.h>
#include <vector>

using namespace std;

struct Ray
{
	D3DXVECTOR3 m_org, m_dir;
};

class OBB
{
public:
	OBB();
	OBB(D3DXVECTOR3 halfSize);
	~OBB();
	void Release();
	void SetSize(D3DXVECTOR3 size);
	void Render(D3DXMATRIX *view, D3DXMATRIX *proj, D3DXVECTOR4 *lightPos, D3DXVECTOR4 *lightColor, D3DXMATRIX *shadow);
	bool Intersect(OBB &b);
	bool Intersect(Ray &rat);
	bool Intersect(D3DXVECTOR3 &point);
	D3DXVECTOR3 GetContactPointWithRay(Ray &ray);
	vector<D3DXVECTOR3> GetAllCornersOfBox();

	D3DXVECTOR3 m_halfSize, m_pos;
	D3DXQUATERNION m_rot;

private:
	ID3DXMesh * m_pMesh;
	float m_HalfDiagonalLength;
	ID3DXEffect *pLightingEffect;
};

#endif