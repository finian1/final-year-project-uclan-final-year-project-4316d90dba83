#pragma once
#include "ComponentAction.h"
class ActionEat :
    public ComponentAction
{
public:
    ActionEat(int parentComponentID)
        :ComponentAction(parentComponentID)
    {
        mActionCost = 0.005f;
    }
    void TriggerAction(Agent* Owner) override;
};

