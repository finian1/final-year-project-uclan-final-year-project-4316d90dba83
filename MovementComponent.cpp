#include "MovementComponent.h"
#include "MovementActions.h"

MovementComponent::MovementComponent(float climbingEfficiency)
{
    movementData = { climbingEfficiency };

    mActions = {
        &gActions.MoveForward,
        &gActions.TurnLeft,
        &gActions.TurnRight,
    };

    mActionTypes = {
        MOVEFORWARD,
        TURNLEFT,
        TURNRIGHT
    };
}
