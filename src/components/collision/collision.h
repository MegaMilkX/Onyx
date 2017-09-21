#ifndef COM_COLLISION_H
#define COM_COLLISION_H

#include <aurora/math.h>
#include "../../scene_object.h"

#include <btBulletDynamicsCommon.h>

class Collider;
class Collision : public SceneObject::Component
{
public:
	Collider* RayTest(Au::Math::Ray& ray)
	{
		Au::Math::Vec3f hit;
		return RayTest(ray, hit);
	}
    
	Collider* RayTest(Au::Math::Ray& ray, Au::Math::Vec3f& hitLocation)
	{
		btVector3 origin = btVector3(ray.origin.x, ray.origin.y, ray.origin.z);
		btVector3 direction = btVector3(ray.direction.x, ray.direction.y, ray.direction.z);
		btVector3 hitPoint;
		btVector3 hitNormal;
		
		btCollisionWorld::ClosestRayResultCallback rayCb(origin, direction);
		world->rayTest(
			origin,
			origin + direction,
			rayCb
		);
		
		if(rayCb.hasHit())
		{
			hitPoint = rayCb.m_hitPointWorld;
			hitPoint.setInterpolate3(origin, origin + direction, rayCb.m_closestHitFraction);
			hitLocation = Au::Math::Vec3f(hitPoint.x(), hitPoint.y(), hitPoint.z());
			hitNormal = rayCb.m_hitNormalWorld;
			const btCollisionObject* co = rayCb.m_collisionObject;
			Collider* c = (Collider*)co->getUserPointer();
			return c;
		}
		
		return 0;
	}
	
	void AddCollider(Collider* col);
    void RemoveCollider(Collider* col);

	virtual void OnCreate()
	{
		collisionConf = new btDefaultCollisionConfiguration();
        dispatcher = new btCollisionDispatcher(collisionConf);
        broadphase = new btDbvtBroadphase();
		
		world = new btCollisionWorld(
            dispatcher, 
            broadphase,
            collisionConf
        );
	}
private:
	btDefaultCollisionConfiguration* collisionConf;
    btCollisionDispatcher* dispatcher;
    btDbvtBroadphase* broadphase;
	btCollisionWorld* world;
};

#endif
