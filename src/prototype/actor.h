#ifndef ACTOR_H
#define ACTOR_H

#include <transform.h>
#include <collision/collider.h>
#include <collision/kinematic_object.h>
#include <skin_mesh.h>
#include <animation.h>
#include <anim_state.h>
#include <skeleton.h>

class Actor : public KinematicObject
{
public:	
	void Velocity(const Au::Math::Vec3f& v)
	{
		if(v.length() > FLT_EPSILON)
		{
			velocity = v;
			animState->Set("velocity", 10.0);
		}
		else if(v.length() <= FLT_EPSILON)
		{
			velocity = Au::Math::Vec3f(0.0f, 0.0f, 0.0f);
			animState->Set("velocity", 0.0);
		}
	}
	
	Au::Math::Vec3f Velocity()
	{
		return velocity;
	}

	void Update(float dt)
	{
		animState->Set("dt", dt);
		animState->Set("direction", velocity);
		_checkForGround();
		//trans->LookAt(trans->Position() - Velocity(), trans->Forward(), Au::Math::Vec3f(0.0f, 1.0f, 0.0f), 10.0f * dt);
		animState->Update();
	}
	
	virtual void OnCreate();
private:
	void _checkForGround()
	{
		Transform* trans = GetComponent<Transform>();
		
		Au::Math::Vec3f pos = trans->Position() + Au::Math::Vec3f(0.0f, 1.0f, 0.0f);
		Collision::RayHit hit;
		if(collision->RayTest(Au::Math::Ray(pos, Au::Math::Vec3f(0.0f, -1.1f, 0.0f)), hit))
		{
			groundHit = hit.position;
			grounded = true;
		}
		else
		{
			grounded = false;
		}
		animState->Set("grounded", grounded);
		animState->Set("groundHit", groundHit);
	}

	AnimState* animState;
	Au::Math::Vec3f velocity;
	Au::Math::Vec3f groundHit;
	bool grounded;
};

#endif
