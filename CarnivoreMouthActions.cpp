#include "CarnivoreMouthActions.h"
#include "CarnivoreMouthComponent.h"
#include "SimGrid.h"
#include "Agent.h"
#include "Cell.h"

//void ActionAttack::TriggerAction(Agent* owner)
//{
//	ComponentAction::TriggerAction(owner);
//	const CarnivoreMouthComponent* component = static_cast<CarnivoreMouthComponent*>(owner->GetComponent(mParentComponentID));
//	utility::Vector2i mLookDir = owner->Directions[owner->mCurrentDirection];
//	int ownerPosX, ownerPosY;
//	owner->GetPosition(ownerPosX, ownerPosY);
//	Cell* attackingCell = owner->GetGrid()->GetCell(ownerPosX + mLookDir.X, ownerPosY + mLookDir.Y);
//	Agent* targetAgent;
//	if (attackingCell->GetOccupyingAgent(targetAgent))
//	{
//		owner->AddEnergy(targetAgent->RemoveEnergy(component->mDamageToDeal) / 2);
//	}
//}
