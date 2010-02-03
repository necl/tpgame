#pragma  once

#include "../../Public/State.h"

template< class entity_name>
class  CGoHomeState : public CState<entity_name>
{

	typedef  entity_type    Entity;

public:

	virtual void Enter(Entity* );

	virtual void Execute(Entity* );

	virtual void Exit(Entity* );

	virtual	bool HandleEvent( eEventAi& e , Entity*) ;

	// return state
	virtual eStateAi GetState()	{	return  Ent_GoHomeState ;	}

private:


};