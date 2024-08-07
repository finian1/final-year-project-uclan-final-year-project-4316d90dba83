#pragma once
#include <string>

class Agent;
class Component;

class ComponentAction
{
public:

	ComponentAction(int parentComponentID)
	{
	}

	virtual void TriggerAction(Agent* Owner);

protected:
	float mActionCost = 0.0f;
};

