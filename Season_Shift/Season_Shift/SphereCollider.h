#pragma once
#include "Collider.h"
#include <typeinfo>
class SphereCollider : public Collider
{
private:
	DirectX::BoundingSphere m_sphere;

	

public:
	SphereCollider(float radius);
	~SphereCollider();

	bool collide(const Ref<Collider>& collider) override;
	void update() override;
	void initialize() override;

	const DirectX::BoundingSphere& getInternalCollider() const;


private:
	template<typename T>
	bool internalCollide(Ref<T> collider) //ugly function to solve problem that i can't solve
	{
		return this->getInternalCollider().Intersects(collider->getInternalCollider());
	}
};
