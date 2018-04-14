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
	typedef std::function<bool(void)> fun_condition_t;
	typedef std::function<void(void)> fun_start_t;
	typedef std::function<void(void)> fun_update_t;

	ActorState()
	: condition(_con_stub), start(_start_stub), update(_update_stub)
	{}
	ActorState(
		const fun_condition_t& con, 
		const fun_start_t& start, 
		const fun_update_t& update,
		const std::vector<std::string>& connected)
	: condition(con), start(start), update(update), connected_states(connected) 
	{}
	fun_condition_t condition;
	fun_start_t start;
	fun_update_t update;
	std::vector<std::string> connected_states;
private:
	static bool _con_stub() { return false; }
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
		for(auto& s : currentState.connected_states)
		{
			if(states[s].condition())
				SwitchState(s);
		}
		currentState.update();	
	}
	
	virtual void OnCreate();
private:
	ActorState currentState;
	std::map<std::string, ActorState> states;
};

#endif
