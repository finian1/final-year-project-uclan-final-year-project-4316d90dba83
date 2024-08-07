#include "MouthActions.h"
#include "MouthComponent.h"
#include "Agent.h"
#include "Cell.h"

//void ActionEat::TriggerAction(Agent* owner)
//{
//	ComponentAction::TriggerAction(owner);
//	const MouthComponent* component = static_cast<MouthComponent*>(owner->GetComponent(mParentComponentID));
//	owner->AddEnergy(owner->GetCell()->RemovePlantNeutrients(component->mAmountToEat));
//}