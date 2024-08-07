#include "MouthComponent.h"
#include "MouthActions.h"

MouthComponent::MouthComponent(float amountToEat)
{
    mActionData = { amountToEat };

    mActions = {
        &gActions.Eat
    };

    mActionTypes = {
        EAT
    };
}