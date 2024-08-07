#pragma once
#include "Globals.h"
#include <vector>
class Agent;
class ComponentInput;
class ComponentAction;

class Component
{
public:

	Component()
	{
		
	}

	~Component()
	{

	}

	virtual ActionData GetActionData()
	{
		return {};
	}

	virtual InputData GetInputData()
	{
		return {};
	}

	/*virtual std::vector<ComponentInput*> GetInputs()
	{
		return mComponentInputs;
	}

	virtual std::vector<ComponentAction*> GetActions()
	{
		return mComponentActions;
	}*/

	virtual std::vector<void (*)(ActionData, Agent*)> GetActions()
	{
		return mActions;
	}

	std::vector<ACTIONTYPE> GetActionTypes()
	{
		return mActionTypes;
	}

	virtual std::vector<float (*)(InputData, Agent*)> GetInputs()
	{
		return mInputs;
	}

	std::vector<INPUTTYPE> GetInputTypes()
	{
		return mInputTypes;
	}

protected:

	std::vector<void (*)(ActionData, Agent*)> mActions;
	std::vector<ACTIONTYPE> mActionTypes;
	std::vector<float (*)(InputData, Agent*)> mInputs;
	std::vector<INPUTTYPE> mInputTypes;
};

