#include "actor.h"

#include "components/camera.h"

typedef Au::Math::Vec3f vec3;
typedef Au::Math::Mat4f mat4;

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

/*
bool ActorState::_checkForGround()
{
	Transform* trans = GetComponent<Transform>();
	Actor* actor = GetComponent<Actor>();
	
	Au::Math::Vec3f pos = trans->Position() + Au::Math::Vec3f(0.0f, 1.0f, 0.0f);
    Collision::RayHit hit;
	if(collision->RayTest(Au::Math::Ray(pos, Au::Math::Vec3f(0.0f, -1.1f, 0.0f)), hit))
	{
		Au::Math::Vec3f pos = trans->Position();
		pos.y = hit.position.y;
		trans->Position(pos);
		return true;
	}
	else
	{
		actor->Switch(GetComponent<ActorFall>());
		return false;
	}
}

void ActorFall::Update(float dt)
{
	Transform* trans = GetComponent<Transform>();
	Actor* actor = GetComponent<Actor>();
	
	velocityFall = -9.8f * dt;
	
	Au::Math::Vec3f pos = trans->Position();
	Collision::RayHit hit;
	if(collision->RayTest(Au::Math::Ray(pos, Au::Math::Vec3f(0.0f, velocityFall - 0.1f, 0.0f)), hit))
	{
		Au::Math::Vec3f pos = trans->Position();
		pos.y = hit.position.y;
		trans->Position(pos);
		actor->Switch(GetComponent<ActorIdle>());
		return;
	}
	
	trans->Translate(0.0f, velocityFall, 0.0f);
}
*/

void Actor::OnCreate()
{
    KinematicObject::OnCreate();

    Collider* collider = GetComponent<SphereCollider>();
    collider->SetOffset(0.0f, 0.6f, 0.0f);
    
	GetComponent<Skeleton>()->SetData("character");
	Model* m = GetComponent<Model>();
	m->mesh.set("character");
	m->material.set("material2");
	
	GetObject()->Name("character");

    Get<Animation>()->Set("character");
    SceneObject* animRoot = Get<Animation>()->Object()->FindObject("Root");
    if(animRoot)
    {
        animRoot->Get<Animation>()->SetRootMotionTarget(Object());
    }
	
    animState = Get<AnimState>();
    animState->AppendScript(R"(
        Idle = {}
        Idle.Start = function()
            State:Blend("Idle", 0.1)
        end
        Idle.Update = function()
            if velocity > 0.0 then
                State:Switch("Walk")
            end
        end
    )");
    animState->AppendScript(R"(
        Walk = {}
        Walk.Start = function()
            State:Blend("Run", 0.1)
        end
        Walk.Update = function()
            if velocity == 0.0 then
                State:Switch("Idle")
            end
        end
    )");
    animState->Set("velocity", 0.0);
    animState->Switch("Idle");
}