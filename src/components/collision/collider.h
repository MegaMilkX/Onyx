#ifndef COM_COLLIDER_H
#define COM_COLLIDER_H

#include "../transform.h"
#include "collision.h"

class Collider : public SceneObject::Component
{
friend Collision;
public:
	~Collider()
	{
		collision->RemoveCollider(this);
	}
	virtual void OnCreate()
	{
		collision = GetObject()->Root()->GetComponent<Collision>();

		collisionObject = new btCollisionObject();		
		collisionObject->setUserPointer(this);
		
		Au::Math::Mat4f mat4f = GetComponent<Transform>()->GetTransform();
		btTransform trans;
        trans.setFromOpenGLMatrix((btScalar*)&mat4f);
		collisionObject->setWorldTransform(trans);
	}
protected:
	btCollisionObject* collisionObject;
	btCollisionShape* shape;
	
	Collision* collision;
};

class PlaneCollider : public Collider
{
public:
	virtual void OnCreate()
	{
		Collider::OnCreate();
		shape = new btStaticPlaneShape(btVector3(0.0f, 1.0f, 0.0f), 0.0f);
		collisionObject->setCollisionShape(shape);
		collision->AddCollider(this);
	}
};

class SphereCollider : public Collider
{
public:
	virtual void OnCreate()
	{
		Collider::OnCreate();
		shape = new btSphereShape(1.0f);
		collisionObject->setCollisionShape(shape);
		collision->AddCollider(this);
	}
};

#endif
