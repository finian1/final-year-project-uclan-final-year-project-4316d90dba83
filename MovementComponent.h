#pragma once

#include "Component.h"

class MovementComponent :
    public Component
{
public:
    MovementComponent(float climbingEfficiency);

    ActionData GetActionData()
    {
        return movementData;
    }

    MovementData movementData;
};

