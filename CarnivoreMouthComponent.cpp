#include "CarnivoreMouthComponent.h"
#include "CarnivoreMouthActions.h"

CarnivoreMouthComponent::CarnivoreMouthComponent(float amountToAttack)
{
    mActionData = { amountToAttack };
    mActions = {
        &gActions.Attack
    };

    mActionTypes = {
        ATTACK
    };
}