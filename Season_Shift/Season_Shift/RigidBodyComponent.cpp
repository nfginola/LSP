#include "RigidBodyComponent.h"
#include <assert.h>


RigidBodyComponent::RigidBodyComponent(float mass)
{
	m_mass = mass;
	assert(m_mass > 0);

	m_force.x = 0;
	m_force.y = 0;
	m_force.z = 0;

	m_acceleration.x = 0;
	m_acceleration.y = 0;
	m_acceleration.z = 0;
	
	m_velocity.x = 0;
	m_velocity.y = 0;
	m_velocity.z = 0;

}

void RigidBodyComponent::setMass(float mass)
{
	m_mass = mass;
}

float RigidBodyComponent::getMass() const
{
	return m_mass;
}