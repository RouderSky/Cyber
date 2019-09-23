#ifndef BONE
#define BONE

#include <d3dx9.h>

struct Bone : public D3DXFRAME
{
	D3DXMATRIX matrixOfbone2Model;		//可从当前骨骼空间转到模型空间的矩阵，由动画更新
};

#endif

