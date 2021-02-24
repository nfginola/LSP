#include "PlayerCameraMovement.h"
#include "Input.h"

using namespace DirectX::SimpleMath;

void PlayerCameraMovement::start()
{
	m_yaw = DirectX::XM_2PI / 2;
	m_pitch = 0.0f;
	m_roll = 0.0f;
	m_goToRoll = 0.0f;

	m_playerCamera = m_gameObject->getComponentType<CameraComponent>(Component::ComponentEnum::CAMERA);
	m_playerCamera->setRotation(m_roll, m_pitch, m_yaw);
}

void PlayerCameraMovement::update()
{
	Input::getInput().mouseMovement(m_pitch, m_yaw);
	m_playerCamera->setRotation(m_roll, m_pitch, m_yaw);

	if (!m_stop)
	{
		if (m_roll < m_goToRoll)
		{
			m_roll += m_frameTime * DirectX::XM_PI * 5.f / 7.f;
		}
		else if (m_roll > m_goToRoll)
		{
			m_roll -= m_frameTime * DirectX::XM_PI * 5.f / 7.f;
		}

		if ((m_goToRoll < 0 && m_goToRoll >= m_roll) || (m_goToRoll > 0 && m_goToRoll <= m_roll))
			m_goToRoll = 0;

		if ((m_direction > 0 && m_roll < 0) || (m_direction < 0 && m_roll > 0))
		{
			if (!m_secondTime)
			{
				setGoToRoll(m_secondTime);
			}
			m_stop = m_secondTime;
			m_roll = 0;
			m_shake = !m_secondTime;
			m_secondTime = !m_secondTime;
		}
	}


	ImGui::Begin("Player Camera");
	{
		ImGui::Text("GoToRoll %f", m_goToRoll);
		ImGui::Text("Roll %f", m_roll);
		ImGui::SliderFloat("Modifier", &temp, 0.01f, 1000.0f);
	}
	ImGui::End();
}

void PlayerCameraMovement::resetCamera()
{
	m_roll = 0.0f;
	m_pitch = 0.0f;
	m_yaw = 0.0f;
	m_goToRoll = 0.0f;
}

void PlayerCameraMovement::updateFrameTime(const float& frameTime)
{
	m_frameTime = frameTime;
}

void PlayerCameraMovement::wallRunning(const bool& wallRunning, const Vector3& normal)
{
	constexpr float minRollOff = 0.01f;
	constexpr float rollWallCheck = 0.3f;
	constexpr float rollModifier = DirectX::XM_PI * 5.f / 7.f;

	if (m_shake)
		return;

	if (!wallRunning)
	{
		if (m_roll > minRollOff)
		{
			m_roll -= m_frameTime;
		}
		else if (m_roll < -minRollOff)
		{
			m_roll += m_frameTime;
		}
		else
		{
			m_roll = 0.0;
		}
	}
	else
	{
		if (m_roll > -rollWallCheck && normal.x > 0)
		{
			m_roll -= normal.x * m_frameTime * rollModifier;
		}
		else if (m_roll < rollWallCheck && normal.x < 0)
		{
			m_roll -= normal.x * m_frameTime * rollModifier;
		}
		if (m_roll > -rollWallCheck && normal.z > 0)
		{
			m_roll -= normal.z * m_frameTime * rollModifier;
		}
		else if (m_roll < rollWallCheck && normal.z < 0)
		{
			m_roll -= normal.z * m_frameTime * rollModifier;
		}
	}
}

void PlayerCameraMovement::shake(Vector3 velocity, const Vector3& normal)
{
	constexpr float minVelocity = -50.0f;
	velocity = normal * velocity;
	if (velocity.y < minVelocity)
	{
		m_stop = false;
		m_shake = true;
		m_secondTime = false;
		setGoToRoll(!m_secondTime);
		m_velocityY = velocity.y;
	}
}

void PlayerCameraMovement::setDirection(const float& roll)
{
	if (roll < 0)
		m_direction = -1;
	else if (roll > 0)
		m_direction = 1;
	else
		m_direction = 0;
}

void PlayerCameraMovement::setGoToRoll(const bool& firstTime)
{
	constexpr float rollAngle = DirectX::XM_PI / 36.0f;
	constexpr float modifier = 100.0f;

	if (firstTime)
	{
		if (m_direction != 0)
			m_goToRoll = rollAngle * m_direction * fabs(m_velocityY) / modifier;
		else
			m_goToRoll = -rollAngle * fabs(m_velocityY) / modifier;
	}
	else
		m_goToRoll = rollAngle * -1 * m_direction * fabs(m_velocityY) / modifier;

	setDirection(m_goToRoll);
}