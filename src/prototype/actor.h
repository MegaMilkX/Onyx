#ifndef ACTOR_H
#define ACTOR_H

#include <transform.h>
#include <collision/collider.h>
#include <collision/kinematic_object.h>
#include <skin_mesh.h>
#include <animator.h>
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
		float dot = Au::Math::Dot(
			Au::Math::Normalize(velocity), 
			Au::Math::Normalize(Get<Transform>()->Back())
		);
		Au::Math::Vec3f rotAxis = Au::Math::Normalize(
			Au::Math::Cross(
				Au::Math::Normalize(Get<Transform>()->Back()), 
				Au::Math::Normalize(velocity)
			)
		);
		#undef max
		#undef min
		const float eps = 0.01f;
    	if(fabs(dot + 1.0f) <= eps)
		{
			
		}
        float rotAngle = acosf(std::max(-1.0f, std::min(dot, 1.0f)));
		if(rotAxis.y > 0.0f) rotAngle *= -1.0f;
		animState->Set("angle", rotAngle);
		animState->Set("angleAbs", fabs(rotAngle));
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
