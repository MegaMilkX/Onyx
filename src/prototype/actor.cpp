#include "actor.h"

#include "components/camera.h"

typedef gfxm::vec3 vec3;
typedef gfxm::mat4 mat4;

class ConvexResultCallback : public btCollisionWorld::ConvexResultCallback
{
public:
    virtual btScalar addSingleResult(btCollisionWorld::LocalConvexResult& convexResult, bool normalInWorldSpace)
    {
        m_closestHitFraction = 0.0f;
        count++;
        return 0.0f;
    }
    
    int count = 0;
};

void Actor::OnCreate()
{
    
	
    
}