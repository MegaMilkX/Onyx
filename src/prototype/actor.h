#ifndef ACTOR_H
#define ACTOR_H

#include <transform.h>
#include <collision/collider.h>
#include <collision/kinematic_object.h>
#include <skin_mesh.h>
#include <animator.h>
#include <motion_script.h>
#include <skeleton.h>

struct ActorState
{
	typedef std::function<void(void)> fun_start_t;
	typedef std::function<void(void)> fun_update_t;

	ActorState()
	: start(_start_stub), update(_update_stub)
	{}
	ActorState(const fun_start_t& start, const fun_update_t& update)
	: start(start), update(update) 
	{}
	fun_start_t start;
	fun_update_t update;
private:
	static void _start_stub() {}
	static void _update_stub() {}
};

class Actor : public SceneObject::Component
{
public:
	void AddState(const std::string& name, const ActorState& state)
	{
		states[name] = state;
	}

	void SwitchState(const std::string& name)
	{
		currentState = states[name];
		currentState.start();
	}

	void Update(float dt)
	{
		currentState.update();	
	}
	
	virtual void OnCreate();
private:
	ActorState currentState;
	std::map<std::string, ActorState> states;
};

#endif
