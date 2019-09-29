#ifndef _ANIMATION_
#define _ANIMATION_

#include <d3dx9.h>

//�������ܴ죬��Ҫ�˰�
class Animation
{
public:
	void init();
	Animation();
	~Animation();
	void Update(float deltaTime);
	void Draw();		//todo���������������
	void OnLostDevice();
	void OnResetDevice();

private:
	ID3DXLine *m_pLine;
	ID3DXKeyframedAnimationSet *m_pAnimSet;
	float m_time;
};

#endif
