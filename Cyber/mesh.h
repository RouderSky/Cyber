#ifndef _MESH_
#define _MESH_

#include <d3dx9.h>
#include <vector>
#include "global.h"

using namespace std;

class Mesh
{
public:
	Mesh();
	Mesh(const char fName[], const char lightingEffectFileName[], const char shadowEffectFileName[]);
	~Mesh();
	HRESULT Load(const char fName[], const char lightingEffectFileName[], const char shadowEffectFileName[]);
	void Render(D3DXMATRIX *view, D3DXMATRIX *proj, D3DXVECTOR4 *lightPos, D3DXVECTOR4 *lightColor, D3DXMATRIX *shadow);
	void DrawMesh();
	void Release();

	D3DXMATRIX world;
private:
	ID3DXMesh * m_pMesh;
	vector<IDirect3DTexture9*> m_textures;
	vector<D3DMATERIAL9> m_materials;
	ID3DXEffect *pLightingEffect;
	ID3DXEffect *pShadowEffect;
};

#endif