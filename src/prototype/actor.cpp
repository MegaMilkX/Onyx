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
    KinematicObject::OnCreate();

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

    
	
    animState = Get<AnimState>();
    animState->AppendScript(R"(
        Idle = {
            Start = function()
                State:Blend("Idle", 0.1)
            end,
            Update = function()
                if not grounded then
                    State:Switch("Fall")
                    return
                end
                if velocity > 0.0 then
                    State:Switch("Walk")
                    return
                end
                vec = Transform:GetPosition()
                Transform:SetPosition(vec.x, groundHit.y, vec.z)
            end
        }
    )");
    animState->AppendScript(R"(
        Walk = {}
        Walk.Start = function()
            State:Blend("Run", 0.1)
            LayerTurnLCur = Animator:GetCursor("LayerTurnL")
            LayerTurnRCur = Animator:GetCursor("LayerTurnR")
            LayerMotion01 = Animator:GetCursor("LayerMotion01")
        end
        Walk.Update = function()
            if not grounded then
                State:Switch("Fall")
                return
            end
            if velocity == 0.0 then
                State:Switch("Idle")
                return
            end
            if angleAbs > 2.0 then
                State:Switch("Turn180")
                return
            end
            vec = Transform:GetPosition()
            Transform:SetPosition(vec.x, groundHit.y, vec.z)
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
    animState->AppendScript(R"(
        Turn180 = {
            Start = function()
                State:Blend("Turn180", 0.1)
            end,
            Update = function()
                if Animator:Stopped(0.15) == 1 then
                    State:Switch("Walk")
                end
            end
        }
    )");
    animState->AppendScript(R"(
        grav_velo = 0.0
        Fall = {}
        Fall.Start = function()
            State:Blend("Bind", 0.1)
        end
        Fall.Update = function()
            if grounded then
                State:Switch("Idle")
                grav_velo = 0.0
                return
            end
            grav_velo = grav_velo + gravity * dt * dt
            Transform:Translate(0.0, -grav_velo, 0.0)
        end
    )");
    animState->Set("velocity", 0.0f);
    animState->Set("gravity", 9.8f);
    animState->Switch("Idle");
}