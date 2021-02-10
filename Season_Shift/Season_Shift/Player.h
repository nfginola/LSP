#pragma once
#include "Logic.h"
#include "Input.h"
#include "CameraComponent.h"

class Player : public Logic
{
private:
	Ref<CameraComponent> m_playerCamera;
	Ref<RigidBody> m_rb;
	float m_pitch, m_yaw, m_roll;

	void lookAround();
	void detectDeath(float death);
	DirectX::SimpleMath::Vector3 respawn;
public:
	Player();
	~Player();

	void start() override;
	void update() override;
	void onCollision(Ref<Collider> collider) override;
	void setRespawn(DirectX::SimpleMath::Vector3 incomingRespawn);
	
};
