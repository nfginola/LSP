#include "GameObject.h"
#include <typeinfo>


GameObject::GameObject()
{
}

GameObject::~GameObject()
{
}




void GameObject::start()
{
}

void GameObject::update()
{
}

int GameObject::AddComponent(Ref<Component> component)
{
	if (typeid(*component) == typeid(Transform))
	{
		if (m_transform == nullptr)
		{
			m_transform = std::dynamic_pointer_cast<Transform>(component);
			if (m_componentVector.size() < 0)
			{
				for (auto &c : m_componentVector)
				{
					c->setTransform(m_transform);
				}
			}
		}
		else
		{
			return -1; // only one transform is allowed
		}
	}
	else
	{
		m_componentVector.push_back(component);
	}
	component->setGameObject(shared_from_this());
	component->setTransform(m_transform);
	return 0;
}

Ref<Transform> GameObject::getTransform()
{
	return m_transform;
}
