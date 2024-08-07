#include "ComponentAction.h"
#include "Agent.h"


void ComponentAction::TriggerAction(Agent* Owner)
{
	Owner->RemoveEnergy(mActionCost);
}
