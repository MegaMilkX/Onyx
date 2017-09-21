#include "actor.h"

bool ActorState::_checkForGround()
{
	Transform* trans = GetComponent<Transform>();
	Actor* actor = GetComponent<Actor>();
	
	Au::Math::Vec3f pos = trans->Position() + Au::Math::Vec3f(0.0f, 1.0f, 0.0f);
	Au::Math::Vec3f hitLoc = trans->Position();
	Collider* collider = collision->RayTest(Au::Math::Ray(pos, Au::Math::Vec3f(0.0f, -1.1f, 0.0f)), hitLoc);
	if(collider)
	{
		Au::Math::Vec3f pos = trans->Position();
		pos.y = hitLoc.y;
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
}


void ActorFall::Update(float dt)
{
	Transform* trans = GetComponent<Transform>();
	Actor* actor = GetComponent<Actor>();
	
	velocityFall = -9.8f * dt;
	
	Au::Math::Vec3f pos = trans->Position();
	Au::Math::Vec3f hitLoc = trans->Position();
	Collider* collider = collision->RayTest(Au::Math::Ray(pos, Au::Math::Vec3f(0.0f, velocityFall - 0.1f, 0.0f)), hitLoc);
	if(collider)
	{
		Au::Math::Vec3f pos = trans->Position();
		pos.y = hitLoc.y;
		trans->Position(pos);
		actor->Switch(GetComponent<ActorIdle>());
		return;
	}
	
	trans->Translate(0.0f, velocityFall, 0.0f);
}

void ActorRun::Update(float dt)
{
	_checkForGround();
	
	trans->Translate(trans->Back() * actor->Velocity().length() * dt);
	trans->LookAt(trans->Position() - actor->Velocity(), Au::Math::Vec3f(0.0f, 1.0f, 0.0f), 10.0f * dt);
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
	GetComponent<Skeleton>()->SetData("character");
	Mesh* m = GetComponent<Mesh>();
	m->SetMesh("character");
	m->SetMaterial("material2");
	
	GetObject()->Name("character");
	
	GetComponent<Animation>()->SetAnim("idle", "character_idle");
	GetComponent<Animation>()->SetAnim("run", "character_run");
	
	Switch(GetComponent<ActorIdle>());
}