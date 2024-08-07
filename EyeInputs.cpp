#include "EyeInputs.h"
#include "Agent.h"
#include "SimGrid.h"
#include "EyeComponent.h"
#include "Cell.h"

//float NutrientSense::GetActivationValue(Agent* owner)
//{
//	int ownerPosX, ownerPosY;
//	const EyeComponent* component = static_cast<EyeComponent*>(owner->GetComponent(mParentComponentID));
//	utility::Vector2i mLookDir = owner->Directions[owner->mCurrentDirection];
//	owner->GetPosition(ownerPosX, ownerPosY);
//	Cell* observedCell = owner->GetGrid()->GetCell(ownerPosX + mLookDir.X + component->mObservationPosX, ownerPosY + mLookDir.Y + component->mObservationPosY);
//	if (observedCell != nullptr)
//	{
//		CellData data = observedCell->GetData();
//		float val = data.mPlantNutrients / data.mMaxNutrients;
//		return(val);
//	}
//	else
//	{
//		return 0.0f;
//	}
//}
//
//float AgentSense::GetActivationValue(Agent* owner)
//{
//	int ownerPosX, ownerPosY;
//	const EyeComponent* component = static_cast<EyeComponent*>(owner->GetComponent(mParentComponentID));
//	utility::Vector2i mLookDir = owner->Directions[owner->mCurrentDirection];
//	owner->GetPosition(ownerPosX, ownerPosY);
//	Cell* observedCell = owner->GetGrid()->GetCell(ownerPosX + mLookDir.X + component->mObservationPosX, ownerPosY + mLookDir.Y + component->mObservationPosY);
//
//	if (observedCell != nullptr && observedCell->HasAgent())
//	{
//		return 1.0f;
//	}
//	else
//	{
//		return 0.0f;
//	}
//}