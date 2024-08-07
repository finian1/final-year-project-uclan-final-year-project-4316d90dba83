#pragma once

#include <string>
#include <vector>

class Agent;
class Component;

class ComponentInput
{
public:

	ComponentInput(int parentComponentID)
	{
		mParentComponentID = parentComponentID;
	}

	virtual float GetActivationValue(Agent* owner)
	{
		return 0.0f;
	}

protected:
	int mParentComponentID = -1;
};

