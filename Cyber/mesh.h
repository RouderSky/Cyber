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
	Mesh(const char fName[]);
	~Mesh();
	HRESULT Load(const char fName[]);
	void Render();
	void Release();
private:
	ID3DXMesh * m_pMesh;
	vector<IDirect3DTexture9*> m_textures;
	vector<D3DMATERIAL9> m_materials;
};

#endif