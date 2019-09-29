#ifndef _ANIMATION_
#define _ANIMATION_

#include <d3dx9.h>

//这个代码很挫，不要了吧
class Animation
{
public:
	void init();
	Animation();
	~Animation();
	void Update(float deltaTime);
	void Draw();		//todo：这个函数不合理
	void OnLostDevice();
	void OnResetDevice();

private:
	ID3DXLine *m_pLine;
	ID3DXKeyframedAnimationSet *m_pAnimSet;
	float m_time;
};

#endif
