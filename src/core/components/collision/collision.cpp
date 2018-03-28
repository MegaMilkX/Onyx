#include "collision.h"

#include "collider.h"
#include "kinematic_object.h"
/*
Au::Math::Vec3f Collision::SweepTest(ConvexGhostCollider* collider, Au::Math::Mat4f& from, Au::Math::Mat4f& to)
{
    btConvexShape* shape = collider->convexShape;
    btTransform f;
    f.setFromOpenGLMatrix((btScalar*)&from);
    btTransform t;
    t.setFromOpenGLMatrix((btScalar*)&to);
    
    btVector3 fromA = btVector3(from[3][0], from[3][1], from[3][2]);
    btVector3 toA = btVector3(to[3][0], to[3][1], to[3][2]);
    
    btCollisionWorld::ClosestConvexResultCallback callback(fromA, toA);
    world->convexSweepTest(shape, f, t, callback);
    
    if(callback.hasHit())
    {
        return Au::Math::Vec3f(callback.m_hitNormalWorld.x(), callback.m_hitNormalWorld.y(), callback.m_hitNormalWorld.z());
    }
    
    return Au::Math::Vec3f();
}
*/
void Collision::AddCollider(Collider* col)
{
	RemoveCollider(col);
    colliders.insert(col);
	world->addCollisionObject(col->collisionObject);
}

void Collision::RemoveCollider(Collider* col)
{
    if(colliders.find(col) == colliders.end())
        return;
    
    colliders.erase(col);
	world->removeCollisionObject(col->collisionObject);
}

void Collision::AddKinematicObject(KinematicObject* ko)
{
    RemoveKinematicObject(ko);
    kinematics.insert(ko);
}

void Collision::RemoveKinematicObject(KinematicObject* ko)
{
    if(kinematics.find(ko) == kinematics.end())
        return;
    kinematics.erase(ko);
}

void Collision::Update(float dt)
{
    std::set<KinematicObject*>::iterator it = kinematics.begin();
    for(it; it != kinematics.end(); ++it)
    {
        (*it)->Update(dt);
    }
    std::set<Collider*>::iterator cit = colliders.begin();
    for(cit; cit != colliders.end(); ++cit)
    {
        (*cit)->UpdateTransform();
    }
    world->performDiscreteCollisionDetection();
    it = kinematics.begin();
    for(it; it != kinematics.end(); ++it)
    {
        (*it)->ResolveCollision();
    }
    cit = colliders.begin();
    for(cit; cit != colliders.end(); ++cit)
    {
        (*cit)->UpdateTransform();
    }
}