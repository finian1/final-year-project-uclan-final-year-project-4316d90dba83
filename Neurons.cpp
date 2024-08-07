#include "Neurons.h"
#include "Brain.h"
#include "Agent.h"
#include "UtilityFunctions.h"

void Neuron::CalculateWeightedSum(Brain* brain)
{
	if (brain->mSimulating)
	{
		HasBeenSimulated = true;
	}
	float val = 0.0f;
	Link currentLinkData;
	for (int i = 0; i < InputLinks.size(); i++)
	{
		currentLinkData = brain->GetLinkData(InputLinks[i]);
		val += currentLinkData.Weight * brain->GetNeuron(currentLinkData.InputNeuron)->Activation;
	}
	val += Bias;
	if (InputLinks.size() == 0)
	{
		Activation = 0.0f;
	}
	else
	{
		Activation = utility::Sigmoid(val);
	}
	SimulationActivationHistory[brain->mTrainingFutureDepth] = Activation;
	if (brain->mSimulating)
	{
		InitialEpisodeActivation = Activation;
	}
}

void ActionNeuron::CalculateWeightedSum(Brain* brain)
{
	if (brain->mSimulating)
	{
		HasBeenSimulated = true;
	}
	//Neuron::CalculateWeightedSum();
	float val = 0.0f;
	Link currentLinkData;
	for (int i = 0; i < InputLinks.size(); i++)
	{
		currentLinkData = brain->GetLinkData(InputLinks[i]);
		val += currentLinkData.Weight * brain->GetNeuron(currentLinkData.InputNeuron)->Activation;
	}
	val += Bias;
	if (InputLinks.size() == 0)
	{
		Activation = 0.0f;
	}
	else
	{
		Activation = utility::Sigmoid(val);
	}
	brain->mCurrentOverallActivationValue += Activation;
	SimulationActivationHistory[brain->mTrainingFutureDepth] = Activation;
	if (brain->mSimulating)
	{
		InitialEpisodeActivation = Activation;
	}
}

void ActionNeuron::TriggerAction(Brain* brain)
{
	NeuronAction(NeuronActionData, brain->GetControllingAgent());
}

void SensorNeuron::CalculateWeightedSum(Brain* brain)
{
	if (brain->mSimulating)
	{
		HasBeenSimulated = true;
	}
	Activation = NeuronInput(NeuronInputData, brain->GetControllingAgent());
	SimulationActivationHistory[brain->mTrainingFutureDepth] = Activation;
	if (brain->mSimulating)
	{
		InitialEpisodeActivation = Activation;
	}
}

float SensorNeuron::TriggerInput(Brain* brain)
{
	return NeuronInput(NeuronInputData, brain->GetControllingAgent());
}
//
//void TestObject::TriggerAction(Brain* input)
//{
//
//}