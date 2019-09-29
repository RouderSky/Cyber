#include "Animation.h"

extern IDirect3DDevice9* pD3DDevice;

Animation::Animation()
{
}

void Animation::init()
{
	m_time = 0.0f;

	//创建一条很粗的线
	D3DXCreateLine(pD3DDevice, &m_pLine);

	//todo：以下可以抽成一个函数
	//D3DXPLAY_PINGPONG模式下，GetPeriod()得到的是2*最大播放秒数
	//D3DXPLAY_LOOP模式下，GetPeriod()得到的是最大播放秒数
	//D3DXPLAY_ONCE模式下，GetPeriod()得到的是最大播放秒数
	D3DXCreateKeyframedAnimationSet("AnimationSet1", 500, D3DXPLAY_PINGPONG, 1, 0, NULL, &m_pAnimSet);

	D3DXKEY_VECTOR3 pos[3];
	pos[0].Time = 0.0f;
	pos[0].Value = D3DXVECTOR3(0.2f, 0.3f, 0.0f);
	pos[1].Time = 1000.0f;
	pos[1].Value = D3DXVECTOR3(0.8f, 0.5f, 0.0f);
	pos[2].Time = 2000.0f;
	pos[2].Value = D3DXVECTOR3(0.4f, 0.8f, 0.0f);

	D3DXKEY_VECTOR3 scale[2];
	scale[0].Time = 500.0f;
	scale[0].Value = D3DXVECTOR3(1.0f, 1.0f, 1.0f);
	scale[1].Time = 1500.0f;
	scale[1].Value = D3DXVECTOR3(4.0f, 4.0f, 4.0f);

	m_pAnimSet->RegisterAnimationSRTKeys("Animation1", 2, 0, 3, scale, NULL, pos, 0);
}

Animation::~Animation()
{
	if (m_pLine != NULL)
		m_pLine->Release();
}

void Animation::Update(float deltaTime)
{
	m_time += deltaTime;
	if (m_time > m_pAnimSet->GetPeriod())		//J...  播放一次的周期
		m_time -= (float)m_pAnimSet->GetPeriod();
}

void Animation::Draw()
{
	D3DVIEWPORT9 vp;
	pD3DDevice->GetViewport(&vp);		//J...

	D3DXVECTOR3 pos, scale;
	D3DXQUATERNION rot;
	m_pAnimSet->GetSRT(m_time, 0, &scale, &rot, &pos);		//J...

	float size = scale.x * 20.0f;
	m_pLine->SetWidth(size);		//J...

	m_pLine->Begin();		//J...

	pos.x *= vp.Width;
	pos.y *= vp.Height;
	D3DXVECTOR2 p[] = { D3DXVECTOR2(pos.x - size * 0.5f, pos.y),
						D3DXVECTOR2(pos.x + size * 0.5f, pos.y) };
	m_pLine->Draw(p, 2, 0xffff0000);	//J...

	m_pLine->End();	//J...
}

void Animation::OnLostDevice()
{
	m_pLine->OnLostDevice();
}

void Animation::OnResetDevice()
{
	m_pLine->OnResetDevice();
}
