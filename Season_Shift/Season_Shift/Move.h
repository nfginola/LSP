#pragma once
#include "Logic.h"

#include <d3d11.h>
#include <SimpleMath.h>

class Move : public Logic
{
private:
	DirectX::SimpleMath::Vector3 m_move;
	DirectX::SimpleMath::Vector3 m_speed;
	DirectX::SimpleMath::Vector3 m_range;
	DirectX::SimpleMath::Vector3 m_original;
	float m_direction;
public:
	Move() = default;
	Move(DirectX::SimpleMath::Vector3 speed, DirectX::SimpleMath::Vector3 range);
	~Move();
	void start() override;
	void update() override;
	DirectX::SimpleMath::Vector3 getSpeed();
};