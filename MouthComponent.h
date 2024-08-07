#pragma once
#include "Component.h"

class MouthComponent :
    public Component
{
public:
    MouthComponent(float amountToEat);

    ActionData GetActionData()
    {
        return mActionData;
    }

    MouthData mActionData;
};

