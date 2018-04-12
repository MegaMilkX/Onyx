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
        actor->Update(dt);

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
        float rotAngle = acosf(std::max(-1.0f, std::min(dot, 1.0f)));
		if(rotAxis.y > 0.0f) rotAngle *= -1.0f;
		motion->Set("angle", rotAngle);
		motion->Set("angleAbs", fabs(rotAngle));
		
		//trans->LookAt(trans->Position() - Velocity(), trans->Forward(), gfxm::vec3(0.0f, 1.0f, 0.0f), 10.0f * dt);
		motion->Update();
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
        Get<Animator>()->Set("character");

        layerMotion1 = asset<Animation>::get("character")->operator[]("LayerMotion01")->GetCursor();
        
        motion = Get<MotionScript>();
        motion->AppendScript(R"(
            Idle = {
                endMargin = 0.1,
                Start = function()
                    State:Blend("Idle", 0.1)
                end,
                Update = function()
                    
                end
            }
        )");
        motion->AppendScript(R"(
            Walk = {}
            Walk.Start = function()
                State:Blend("Run", 0.1)
                LayerTurnLCur = Animator:GetCursor("LayerTurnL")
                LayerTurnRCur = Animator:GetCursor("LayerTurnR")
                LayerMotion01 = Animator:GetCursor("LayerMotion01")
            end
            Walk.Update = function()
                if angleAbs > 2.0 then
                    State:Switch("Turn180")
                    return
                end
                LayerTurnLCur:Advance(dt)
                LayerTurnRCur:Advance(dt)
                if angle > 0.0 then
                    Animator:ApplyAdd(LayerTurnRCur, angle / 3.0)    
                end
                if angle < 0.0 then
                    Animator:ApplyAdd(LayerTurnLCur, -angle / 3.0)
                end
            end
        )");
        motion->AppendScript(R"(
            Turn180 = {
                Start = function()
                    State:Blend("Turn180", 0.1)
                end,
                Update = function()
                    if Animator:Stopped(0.15) == 1 then
                        State:Switch("Walk")
                    end
                end,
                End = function()
                    State:Switch("Walk")
                end
            }
        )");
        motion->AppendScript(R"(
            Fall = {}
            Fall.Start = function()
                State:Blend("Bind", 0.1)
            end
            Fall.Update = function()
                
            end
        )");
        motion->Set("velocity", 0.0f);
        motion->Set("gravity", 9.8f);

        actor = Get<Actor>();
        actor->AddState(
            "Idle",
            {
                [this](){
                    motion->Switch("Idle");
                },
                [this](){
                    _checkForGround();
                    if(!grounded)
                    {
                        actor->SwitchState("Fall");
                        return;
                    }
                    if(velocity.length() > FLT_EPSILON)
                    {
                        actor->SwitchState("Walk");
                        return;
                    }
                }
            }
        );
        actor->AddState(
            "Walk",
            {
                [this](){
                    motion->Switch("Walk");
                },
                [this](){
                    _checkForGround();
                    if(!grounded)
                    {
                        actor->SwitchState("Fall");
                        return;
                    }
                    if(velocity.length() <= FLT_EPSILON)
                    {
                        actor->SwitchState("Idle");
                        return;
                    }
                    
                }
            }
        );
        actor->AddState(
            "Fall",
            {
                [this](){
                    motion->Switch("Fall");
                },
                [this](){
                    _checkForGround();
                    if(grounded)
                    {
                        actor->SwitchState("Idle");
                        grav_velo = 0.0f;
                        return;
                    }
                    grav_velo += 9.8f * GameState::DeltaTime() * GameState::DeltaTime();
                    Get<Transform>()->Translate(0.0, -grav_velo, 0.0);
                }
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

    AnimTrack::Cursor layerMotion1;

    Actor* actor;
    KinematicObject* kinematicObject;
    Collision* collision;
    MotionScript* motion;

	gfxm::vec3 velocity;
	gfxm::vec3 groundHit;
	bool grounded;
    float grav_velo = 0.0f;
};

#endif
