#pragma once
#include "Globals.h"
#include "Component.h"
class EyeComponent :
    public Component
{
public:
    EyeComponent(int ObsPosX, int ObsPosY);

    InputData GetInputData()
    {
        return mInputData;
    }

    EyeData mInputData;
};

