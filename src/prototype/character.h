#ifndef CHARACTER_H
#define CHARACTER_H

#include "actor.h"

class Character : public SceneObject::Component
{
public:
    void Velocity(const gfxm::vec3& v)
	{
		if(v.length() > FLT_EPSILON)
		{
			velocity = v;
			motion->Set("velocity", 10.0);
		}
		else if(v.length() <= FLT_EPSILON)
		{
			velocity = gfxm::vec3(0.0f, 0.0f, 0.0f);
			motion->Set("velocity", 0.0);
		}
	}
	
	gfxm::vec3 Velocity()
	{
		return velocity;
	}

    void Update(float dt)
    {
        motion->Set("dt", dt);
		motion->Set("direction", velocity);
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
        rotAngle = acosf(std::max(-1.0f, std::min(dot, 1.0f)));
		if(rotAxis.y > 0.0f) rotAngle *= -1.0f;
		motion->Set("angle", rotAngle);
		motion->Set("angleAbs", fabs(rotAngle));

        angleAbs = fabs(rotAngle);
		
		//trans->LookAt(trans->Position() - Velocity(), trans->Forward(), gfxm::vec3(0.0f, 1.0f, 0.0f), 10.0f * dt);
		animator->Tick(dt);
        actor->Update(dt);
        animator->Finalize();
        
        //motion->Update();
		layerMotion1 += dt;
		Get<Animator>()->ApplyAdd(layerMotion1, 1.0f);
    }

    void OnCreate()
    {
        collision = RootGet<Collision>();
        kinematicObject = Get<KinematicObject>();

        Collider* collider = GetComponent<SphereCollider>();
        collider->SetOffset(0.0f, 0.6f, 0.0f);
        
        GetComponent<Skeleton>()->SetData("character");
        Model* m = GetComponent<Model>();
        m->mesh.set("character");
        m->material.set("material2");
        
        GetObject()->Name("character");

        asset<Animation>::get("character")->SetRootMotionSource("Root");
        asset<Animation>::get("character")->operator[]("Turn180")->Looping(false);
        animator = Get<Animator>();
        animator->Set("character");

        layerMotion1 = asset<Animation>::get("character")->operator[]("LayerMotion01")->GetCursor();
        
        motion = Get<MotionScript>();
        
        motion->Set("velocity", 0.0f);
        motion->Set("gravity", 9.8f);

        actor = Get<Actor>();
        actor->AddState(
            "Idle",
            {
                [this]()->bool{
                    return velocity.length() <= FLT_EPSILON;
                },
                [this](){
                    grav_velo = 0.0f;
                    motion->Blend("Idle", 0.1f);
                },
                [this](){
                    _checkForGround();
                },
                { "Fall", "Walk" }
            }
        );
        actor->AddState(
            "Walk",
            {
                [this]()->bool{
                    return velocity.length() > FLT_EPSILON;
                },
                [this](){
                    grav_velo = 0.0f;
                    motion->Blend("Run", 0.1f);
                    LayerTurnLCur = Get<Animator>()->GetAnimCursor("LayerTurnL");
                    LayerTurnRCur = Get<Animator>()->GetAnimCursor("LayerTurnR");
                },
                [this](){
                    _checkForGround();

                    LayerTurnLCur.Advance(GameState::DeltaTime());
                    LayerTurnRCur.Advance(GameState::DeltaTime());
                    if (rotAngle > 0.0f)
                        Get<Animator>()->ApplyAdd(LayerTurnRCur, rotAngle / 3.0f);    
                    if (rotAngle < 0.0f)
                        Get<Animator>()->ApplyAdd(LayerTurnLCur, -rotAngle / 3.0f);
                },
                { "Fall", "Idle", "Turn180" }
            }
        );
        actor->AddState(
            "Turn180",
            {
                [this]()->bool{
                    return angleAbs > 2.0f;
                },
                [this](){
                    motion->Blend("Turn180", 0.1f);
                },
                [this](){
                    if(Get<Animator>()->Stopped(0.15f))
                        actor->SwitchState("Walk");
                },
                {  }
            }
        );
        actor->AddState(
            "Fall",
            {
                [this]()->bool{
                    return !grounded;
                },
                [this](){
                    motion->Blend("Bind", 0.1f);
                },
                [this](){
                    _checkForGround();
                    grav_velo += 9.8f * GameState::DeltaTime() * GameState::DeltaTime();
                    Get<Transform>()->Translate(0.0, -grav_velo, 0.0);
                },
                { "Idle" }
            }
        );
        actor->SwitchState("Idle");        
    }
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
            gfxm::vec3 vec = trans->Position();
            trans->Position(vec.x, groundHit.y, vec.z);
		}
		else
		{
			grounded = false;
		}
		motion->Set("grounded", grounded);
		motion->Set("groundHit", groundHit);
	}

    AnimTrack::Cursor LayerTurnLCur;
    AnimTrack::Cursor LayerTurnRCur;
    AnimTrack::Cursor layerMotion1;

    Actor* actor;
    KinematicObject* kinematicObject;
    Collision* collision;
    MotionScript* motion; // MotionFlow ?
    Animator* animator;

    float rotAngle;
    float angleAbs;
	gfxm::vec3 velocity;
	gfxm::vec3 groundHit;
	bool grounded;
    float grav_velo = 0.0f;
};

#endif
