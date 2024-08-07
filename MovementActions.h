#pragma once
#include "ComponentAction.h"

class ActionMoveForward :
    public ComponentAction
{
public:
    ActionMoveForward(int parentComponentID)
        :ComponentAction(parentComponentID)
    {
        mActionCost = 0.01f;
    }
    void TriggerAction(Agent* Owner) override;
};

class ActionMoveBackward :
    public ComponentAction
{
public:
    ActionMoveBackward(int parentComponentID)
        :ComponentAction(parentComponentID)
    {
        mActionCost = 0.01f;
    }
    void TriggerAction(Agent* Owner) override;
};

class ActionTurnLeft :
    public ComponentAction
{
public:
    ActionTurnLeft(int parentComponentID)
        :ComponentAction(parentComponentID)
    {
        mActionCost = 0.01f;
    }
    void TriggerAction(Agent* Owner) override;
};

class ActionTurnRight :
    public ComponentAction
{
public:
    ActionTurnRight(int parentComponentID)
        :ComponentAction(parentComponentID)
    {
        mActionCost = 0.01f;
    }
    void TriggerAction(Agent* Owner) override;
};
