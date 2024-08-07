#pragma once
#include "Component.h"
class CarnivoreMouthComponent :
    public Component
{

public:
    CarnivoreMouthComponent(float damage);

    ActionData GetActionData()
    {
        return mActionData;
    }

    AttackData mActionData;
};

