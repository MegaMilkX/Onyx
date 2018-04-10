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
	void Velocity(const gfxm::vec3& v)
	{
		if(v.length() > FLT_EPSILON)
		{
			velocity = v;
			animState->Set("velocity", 10.0);
		}
		else if(v.length() <= FLT_EPSILON)
		{
			velocity = gfxm::vec3(0.0f, 0.0f, 0.0f);
			animState->Set("velocity", 0.0);
		}
	}
	
	gfxm::vec3 Velocity()
	{
		return velocity;
	}

	void Update(float dt)
	{
		

		animState->Set("dt", dt);
		animState->Set("direction", velocity);
		float dot = gfxm::dot(
			gfxm::normalize(velocity), 
			gfxm::normalize(Get<Transform>()->Back())
		);
		gfxm::vec3 rotAxis = gfxm::normalize(
			gfxm::cross(
				gfxm::normalize(Get<Transform>()->Back()), 
				gfxm::normalize(velocity)
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
		//trans->LookAt(trans->Position() - Velocity(), trans->Forward(), gfxm::vec3(0.0f, 1.0f, 0.0f), 10.0f * dt);
		animState->Update();
		
	}
	
	virtual void OnCreate();
private:
	void _checkForGround()
	{
		Transform* trans = GetComponent<Transform>();
		
		gfxm::vec3 pos = trans->Position() + gfxm::vec3(0.0f, 1.0f, 0.0f);
		Collision::RayHit hit;
		if(collision->RayTest(gfxm::ray(pos, gfxm::vec3(0.0f, -1.1f, 0.0f)), hit))
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
	gfxm::vec3 velocity;
	gfxm::vec3 groundHit;
	bool grounded;
};

#endif
