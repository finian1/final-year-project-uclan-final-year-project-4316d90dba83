#include "MovementActions.h"
#include "Agent.h"

void ActionMoveForward::TriggerAction(Agent* Owner)
{
	ComponentAction::TriggerAction(Owner);
	utility::Vector2i ownerDir = Owner->Directions[Owner->mCurrentDirection];
	Owner->Move(ownerDir.X, ownerDir.Y);
}

void ActionMoveBackward::TriggerAction(Agent* Owner)
{
	ComponentAction::TriggerAction(Owner);
	utility::Vector2i ownerDir = Owner->Directions[Owner->mCurrentDirection];
	Owner->Move(-ownerDir.X, -ownerDir.Y);
}

void ActionTurnRight::TriggerAction(Agent* Owner)
{
	ComponentAction::TriggerAction(Owner);
	Owner->mCurrentDirection = (Owner->mCurrentDirection + 1) % 4;
}

void ActionTurnLeft::TriggerAction(Agent* Owner)
{
	ComponentAction::TriggerAction(Owner);
	if (Owner->mCurrentDirection - 1 < 0)
	{
		Owner->mCurrentDirection = 3;
	}
	else
	{
		Owner->mCurrentDirection--;
	}
}