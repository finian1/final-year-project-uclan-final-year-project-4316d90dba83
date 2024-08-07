#include "EyeComponent.h"
#include "EyeInputs.h"

EyeComponent::EyeComponent(int ObsPosX, int ObsPosY)
{
	mInputData = { ObsPosX, ObsPosY };
	/*mComponentInputs = {
		new NutrientSense(mComponentID),
		new AgentSense(mComponentID)
	};*/
	mInputs = {
		&gInputs.NutrientSense,
		&gInputs.AgentSense,
		&gInputs.SolidSense,
		&gInputs.HeightSense,
		&gInputs.LavaSense,
		&gInputs.PoisonSense
	};
	mInputTypes = {
		NUTRIENTSENSE,
		AGENTSENSE,
		SOLIDSENSE,
		HEIGHTSENSE,
		LAVASENSE,
		POISONSENSE
	};
}