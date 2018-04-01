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

void ActorIdle::Update(float dt)
{
	_checkForGround();
    
    SceneObject* o = GetObject()->FindObject("Head");
    if(o)
    {
        Transform* t = o->GetComponent<Transform>();
        Au::Math::Vec3f dir = GetObject()->Root()->GetComponent<Renderer>()->CurrentCamera()->GetComponent<Transform>()->Forward();
        //t->LookAtChain(t->WorldPosition() - dir, trans->Forward(), vec3(0.0f, 1.0f, 0.0f), 0.3f, 3);
        //t->Track(vec3(0.0f, 0.0f, 0.0f), vec3(0.0f, 1.0f, 0.0f));
    }
    o = GetObject()->FindObject("Palm.L");
    if(o)
    {
        Transform* t = o->GetComponent<Transform>();
        //t->FABRIK(vec3(0.0f, 1.0f, 1.0f), 3);
        //t->Track(vec3(0.0f, 1.3f, 0.0f));
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

void ActorRun::Update(float dt)
{
    Collider* collider = GetComponent<SphereCollider>();
    collider->SetOffset(0.0f, 0.6f, 0.0f);
    
    trans->LookAt(trans->Position() - actor->Velocity(), trans->Forward(), Au::Math::Vec3f(0.0f, 1.0f, 0.0f), 10.0f * dt);
    
    SceneObject* o = GetObject()->FindObject("Head");
    if(o)
    {
        Transform* t = o->GetComponent<Transform>();
        //t->LookAt(t->WorldPosition() - actor->Velocity(), trans->Forward(), vec3(0.0f, 1.0f, 0.0f), 5.0f * dt);
        //t->LookAtChain(t->WorldPosition() - vec3(0.0f, 0.5f, 0.0f) - actor->Velocity(), trans->Forward(), vec3(0.0f, 1.0f, 0.0f), 15.0f * dt, 3);
        //t->Track(vec3(0.0f, 0.0f, 0.0f), vec3(0.0f, 1.0f, 0.0f));
    }
    
    o = GetObject()->FindObject("Palm.L");
    if(o)
    {
        Transform* t = o->GetComponent<Transform>();
        //t->FABRIK(vec3(0.0f, 1.0f, 1.0f), 3);
        //t->Track(vec3(0.0f, 1.3f, 0.0f));
    }
    
    vec3 pos = trans->Position();
    vec3 dir = Au::Math::Normalize(trans->Back());
    vec3 velo = dir * actor->Velocity().length() * dt;
    vec3 tgtPos = pos + velo;
    
    float stepHeight = 0.25f;
    float slopeMax = 0.25f;
    
    Collision::RayHit hit;
    if(collision->RayTest(Au::Math::Ray(tgtPos + vec3(0.0f, stepHeight, 0.0f), vec3(0.0f, -stepHeight - 0.25f, 0.0f)), hit))
    {
        tgtPos = hit.position;
        velo = (tgtPos - pos) * actor->Velocity().length() * dt;
    }
    
    //trans->Position(tgtPos);
    
    _checkForGround();
    
    /*
    btCollisionWorld::ContactResultCallback callback;
    collision->GetBtWorld()->contactTest(GetComponent<SphereCollider>()->GetBtObject(), callback);
    if(callback.needsCollision())
    {
        
    }
    */
    /*
    mat4 from = trans->GetTransform();    
    from = Au::Math::Translate(from, vec3(0.0f, 0.6f, 0.0f));
    mat4 to = from;
    to = Au::Math::Translate(to, (tgtPos - pos));
    
    hitNormalTransform->SetTransform(to);
    hitNormalTransform->Translate((tgtPos - pos) * 0.5f);
    
    btTransform f;
    f.setFromOpenGLMatrix((btScalar*)&from);
    btTransform t;
    t.setFromOpenGLMatrix((btScalar*)&to);
    
    btVector3 fromA = btVector3(from[3][0], from[3][1], from[3][2]);
    btVector3 toA = btVector3(to[3][0], to[3][1], to[3][2]);
    
    ConvexResultCallback callback;
    
    btSphereShape shape(0.5f);
    collision->GetBtWorld()->convexSweepTest(&shape, f, t, callback);
    
    if(callback.hasHit())
    {*/
        /*
        btVector3 btPos;
        btPos.setInterpolate3(
            callback.m_convexFromWorld,
            callback.m_convexToWorld,
            callback.m_closestHitFraction
        );
        
        hitNormalTransform->Position(btPos.x(), btPos.y(), btPos.z());
        
        vec3 hitNormal(callback.m_hitNormalWorld.x(), callback.m_hitNormalWorld.y(), callback.m_hitNormalWorld.z());
        hitNormal.y = 0.0f;
        //hitNormalTransform->LookAt(hitNormalTransform->Position() + hitNormal, vec3(0.0f, 1.0f, 0.0f));
        
        //hitNormal = Au::Math::Normalize(hitNormal);
        //if(Au::Math::Dot(dir, hitNormal) < 0.5f)
        {
            velo = hitNormal;// - dir;
            //velo.y = 0.0f;
            velo = velo * actor->Velocity().length() * dt;
            tgtPos = pos + velo;
            trans->Position(tgtPos);
        }
        trans->Position(tgtPos);
        //trans->Position(btPos.x(), btPos.y(), btPos.z());
        */
        /*
    }
    else
    {
        trans->Position(tgtPos);
    }
    */
    
    
    //_checkForGround();
}

void ActorIdle::Velocity(const Au::Math::Vec3f& v)
{
	if(v.length() > FLT_EPSILON)
		GetComponent<Actor>()->Switch(GetComponent<ActorRun>());
}

void ActorRun::Velocity(const Au::Math::Vec3f& v)
{
	if(v.length() <= FLT_EPSILON)
		GetComponent<Actor>()->Switch(GetComponent<ActorIdle>());
}

void Actor::OnCreate()
{
    KinematicObject::OnCreate();
    
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
	
	//GetComponent<Animation>()->SetAnim("idle", "character_idle");
	//GetComponent<Animation>()->SetAnim("run", "character_run");
	
	Switch(GetComponent<ActorIdle>());
}