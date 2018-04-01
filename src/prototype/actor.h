#ifndef ACTOR_H
#define ACTOR_H

#include <transform.h>
#include <collision/collider.h>
#include <collision/kinematic_object.h>
#include <skin_mesh.h>
#include <animation.h>
#include <skeleton.h>

class Actor;
class ActorState : public SceneObject::Component
{
public:
	virtual void Switch() = 0;
	virtual void Velocity(const Au::Math::Vec3f& v) {}
	virtual void Update(float dt) = 0;
	virtual void OnCreate()
	{
		trans = GetComponent<Transform>();
		actor = GetComponent<Actor>();
		collision = GetObject()->Root()->GetComponent<Collision>();
	}
protected:
	bool _checkForGround();
	Actor* actor;
	Transform* trans;
	Collision* collision;
};

class Actor : public KinematicObject
{
public:
	void Switch(ActorState* s)
	{
		state = s;
		state->Switch();
	}
	
	void Velocity(const Au::Math::Vec3f& v)
	{
		state->Velocity(v);
		velocity = v;
	}
	
	Au::Math::Vec3f Velocity()
	{
		return velocity;
	}

	void Update(float dt)
	{
		state->Update(dt);
	}
	
	virtual void OnCreate();
private:
	ActorState* state;
	Au::Math::Vec3f velocity;
};

class ActorIdle : public ActorState
{
public:
	virtual void Switch()
	{
		GetComponent<Animation>()->BlendOverTime("Idle", 0.1f);
	}
	
	virtual void Velocity(const Au::Math::Vec3f& v);
	virtual void Update(float dt);
};

class ActorFall : public ActorState
{
public:
	ActorFall()
	: velocityFall(0.0f) {}
	virtual void Switch() 
	{
		velocityFall = 0.0f;
	}
	virtual void Update(float dt);
private:
	float velocityFall;
};

class ActorRun : public ActorState
{
public:
	virtual void Switch()
	{
		GetComponent<Animation>()->BlendOverTime("Run", 0.1f);
	}
	
	virtual void Velocity(const Au::Math::Vec3f& v);
	virtual void Update(float dt);
    
    virtual void OnCreate()
    {
        ActorState::OnCreate();
        SceneObject* o = GetObject()->CreateObject();
        o->Name("ORIENT");
        hitNormalTransform = o->GetComponent<Transform>();
        //hitNormalTransform->AttachTo(GetComponent<Transform>());
        hitNormalTransform->Translate(0.0f, 1.0f, 0.0f);
    }
private:
    Transform* hitNormalTransform;
};

#endif
