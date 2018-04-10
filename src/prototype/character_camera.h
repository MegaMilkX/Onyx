#ifndef CHARACTER_CAMERA_H
#define CHARACTER_CAMERA_H

#include <collision/collider.h>
#include <transform.h>
#include <sound_emitter.h>
#include <camera.h>

class CharacterCamera : public SceneObject::Component
{
public:
    void SetTarget(SceneObject* so)
    { target = so->GetComponent<Transform>(); }
    void SetTarget(SceneObject::Component* com)
    { target = com->GetComponent<Transform>(); }

    void MouseMove(int x, int y)
    {
        transform->Rotate(-x * 0.005f, 0.0f, 1.0f, 0.0f);
        transform->Rotate(-y * 0.005f, transform->Right());
    }
    
    event_dispatcher<eMouseMove> disp_onMouseMove;
    void Update(float dt)
    {
        while(eMouseMove* e = disp_onMouseMove.poll())
            MouseMove(e->dx, e->dy);
        
        gfxm::vec3 tgt = target->Position();
        tgt.y += 1.5f;
        tgt = (tgt - transform->Position()) * (dt * 7.0f);
        transform->Translate(tgt);
        
        Collision::RayHit hit;
        if(GetObject()->Root()->GetComponent<Collision>()->RayTest(gfxm::ray(transform->Position(), transform->Back() * 1.2f), hit))
        {
            cam->GetComponent<Transform>()->Position(0.0f, 0.0f, (hit.position - transform->Position()).length() - 0.1f);
        }
        else
        {
            cam->GetComponent<Transform>()->Position(0.0f, 0.0f, 1.2f);
        }
    }
    
    void OnCreate()
    {
        transform = GetComponent<Transform>();
        target = GetObject()->Root()->GetComponent<Transform>();
        
        cam = GetObject()->CreateObject()->GetComponent<Camera>();
        cam->Perspective(1.4f, 16.0f/9.0f, 0.01f, 1000.0f);
        cam->GetComponent<Transform>()->Translate(0.0, 0.0, 1.2f);
        cam->GetComponent<Transform>()->AttachTo(transform);
        
        cam->GetComponent<SoundListener>();
    }
private:
    Transform* transform;
    Transform* target;
    Camera* cam;
};

#endif
