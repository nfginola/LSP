#include "Player.h"

using namespace DirectX::SimpleMath;

 Player::Player()
 {
	 m_yaw = DirectX::XM_2PI/2;
	 m_pitch = 0.0f;
	 m_roll = 0.0f;
	 respawn = { 0, 0, 0 };
	 m_disable = false;
	 m_frameTime = 0.0f;
	 m_speed = 5000.0f;
 }

 Player::~Player()
 {

 }

 void Player::start()
 {
	 m_playerCamera = m_gameObject->getComponentType<CameraComponent>(Component::ComponentEnum::CAMERA);
	 m_playerCamera->setOffset(0, 3.0f, -0.5);
	 m_rb = m_gameObject->getComponentType<RigidBody>(Component::ComponentEnum::RIGID_BODY);
	 m_playerCamera->setRotation(m_roll, m_pitch, m_yaw);
 }	

 Vector3 antiMovement(Vector3 velocity)
 {
	 if (velocity.Length() > 0.1f)
	 {
		 Vector3 velocityNormal = velocity;
		 velocityNormal.Normalize();
		 velocity -= velocityNormal * 0.1f * velocity.Length();
	 }
	 return velocity;
 }

 void Player::update()
 {
	lookAround();
	detectDeath(-35.0f);
	Vector3 velocity = m_rb->getVelocity();
	Vector3 cameraForward = m_playerCamera->getForward();
	Vector3 cameraRight = m_playerCamera->getRight();
	
	if (Input::getInput().keyPressed(Input::C))
	{
		if (m_disable == false)
		{
			m_disable = true;
		}
		else
		{
			m_disable = false;
		}
	}
	if (m_disable == false)
	{
		if (Input::getInput().keyBeingPressed(Input::W))
		{
			velocity += cameraForward *m_frameTime * m_speed;
		}
		if (Input::getInput().keyBeingPressed(Input::S))
		{
			velocity -= cameraForward *m_frameTime * m_speed;
		}
		if (Input::getInput().keyBeingPressed(Input::A))
		{
			velocity -= cameraRight *m_frameTime * m_speed;
		}
		if (Input::getInput().keyBeingPressed(Input::D))
		{
			velocity += cameraRight *m_frameTime * m_speed;
		}
		if (Input::getInput().keyPressed(Input::Esc))
		{
			exit(0);
		}
		if (Input::getInput().keyPressed(Input::Space))
		{
			velocity += Vector3(0,  7, 0);
		}
		if (Input::getInput().keyPressed(Input::Shift))
		{
			velocity -= Vector3(0, 50, 0);
			//m_rb->addForce(Vector3(0, -50, 0));
		}
	}
	if (Input::getInput().keyPressed(Input::L))
	{
		Input::getInput().lockMouse();
	}
	

	Vector3 velocitySkipY = velocity;
	velocitySkipY.y = 0;
	velocitySkipY = antiMovement(velocitySkipY);

	//velocitySkipY = velocity;
	velocitySkipY.y = 0;
	if (velocitySkipY.Length() > 20.0f)
	{
		Vector3 velocityNormal = velocitySkipY;
		velocityNormal.Normalize();
		velocitySkipY = velocityNormal * 20.0f;
	}
	velocitySkipY.y = velocity.y;
	velocity = velocitySkipY;

	m_rb->setVelocity(velocity);
	m_playerCamera->update();
 }

 void Player::onCollision(Ref<Collider> collider)
 {
	
 }

 void Player::lookAround() 
 {
	 Input::getInput().mouseMovement(m_pitch, m_yaw);
	 m_playerCamera->setRotation(m_roll, m_pitch, m_yaw);
 }

 void Player::detectDeath(float death) 
 {
	 if (m_rb->getTransform()->getPosition().y < death)
	 {
		 m_rb->getTransform()->setPosition(respawn);
	 }
 }

 void Player::setRespawn(Vector3 incomingRespawn)
 {
	 respawn = incomingRespawn;
 }

 void Player::setFrametime(long double dt)
 {
	 m_frameTime = dt;
 }