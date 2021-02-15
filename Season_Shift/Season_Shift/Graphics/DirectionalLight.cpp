#include "DirectionalLight.h"

using namespace DirectX::SimpleMath;

DirectionalLight::DirectionalLight()
	: m_direction(Vector3(0, 0, 1)), m_color(Vector3(1, 1, 1)), m_ambIntensity(0.1)
{}

DirectionalLight::DirectionalLight(Vector3 direction, Vector3 color, float ambientIntensity)
	: m_direction(direction), m_color(color), m_ambIntensity(ambientIntensity)
{}

DirectionalLight::~DirectionalLight()
{}

const DirectX::SimpleMath::Vector3& DirectionalLight::getDirection()
{
	return m_direction;
}

const DirectX::SimpleMath::Vector3& DirectionalLight::getColor()
{
	return m_color;
}

const float DirectionalLight::getIntensity()
{
	return m_ambIntensity;
}

DirectionalLight::DirLight DirectionalLight::getLight()
{
	Vector4 col = { m_color.x, m_color.y, m_color.z, 1 };
	return { m_direction, m_ambIntensity, col };
}