#pragma once
#include "ComponentAction.h"

class ActionAttack
	: public ComponentAction
{
public:
    ActionAttack(int parentComponentID)
        :ComponentAction(parentComponentID)
    {
        mActionCost = 0.005f;
    }
    void TriggerAction(Agent* Owner) override;
};

