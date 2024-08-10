#pragma once

#include <string>
#include <vector>

class Agent;
class Component;

class ComponentInput
{
public:

	ComponentInput()
	{
	}

	virtual float GetActivationValue(Agent* owner)
	{
		return 0.0f;
	}
};

