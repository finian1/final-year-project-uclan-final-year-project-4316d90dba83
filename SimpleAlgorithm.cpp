#include "SimpleAlgorithm.h"
#include "Brain.h"
#include "Agent.h"
#include "UtilityTypes.h"

void SimpleAlgorithm::AdjustWeights(Brain& brain)
{
	float currentReward = brain.mReward;
	float probability = brain.mLastTriggeredNeuron->Activation / brain.mCurrentOverallActivationValue;
	//We need to: Simulate the future time steps that the agent would most likely take based on the new state
	//Add the reward gained based on these future steps, adjusted by the discount factor

	//Data that needs to be reset after predictions:
	//Agent position
	//Agent energy
	utility::Vector2i initialPos = brain.mControllingAgent->GetPosition();
	float initialEnergy = brain.mControllingAgent->GetEnergy();
	float initialOverallActivationValue = brain.mCurrentOverallActivationValue;
	brain.mSimulating = true;
	for (int i = 1; i < brain.mFutureDepth + 1; i++)
	{
		brain.mCurrentOverallActivationValue = 0.0f;
		brain.CalculateOutputWeights();
		brain.ActivateHighestActionNeuron();
		//Reward for the current action should be: diff in energy, and diff in how many nutrients are around the agent.
		brain.mReward = brain.mControllingAgent->GetActionReward() * pow(brain.mDiscountFactor, i);
		currentReward += brain.mReward;
	}
	brain.mValue = currentReward;
	brain.mSimulating = false;
	brain.mControllingAgent->SetPosition(initialPos);
	brain.mControllingAgent->SetEnergy(initialEnergy);
	brain.mCurrentOverallActivationValue = initialOverallActivationValue;

	float L = 0;
	L = brain.mValue * log(probability);

	//TODO: optimize policy using L
	//At least this time it's not *worse* than complete random, but it's not quite right.
	Neuron* triggeredNeuron = brain.mLastTriggeredNeuron;
	std::queue<Neuron*> mNeuronsToCheck({ triggeredNeuron });
	std::queue<int> mLinksToShift;
	while (mNeuronsToCheck.size() != 0)
	{
		Neuron* currentNeuron = mNeuronsToCheck.front();
		currentNeuron->Activation = currentNeuron->InitialEpisodeActivation;
		mNeuronsToCheck.pop();
		for (int i = 0; i < currentNeuron->InputLinks.size(); i++)
		{
			Link data = brain.GetLinkData(currentNeuron->InputLinks[i]);
			mLinksToShift.push(data.ID);
			Neuron* inputNeuron = brain.GetNeuron(data.InputNeuron);
			mNeuronsToCheck.push(inputNeuron);
		}
	}
	while (mLinksToShift.size() != 0)
	{
		Link data = brain.GetLinkData(mLinksToShift.front());
		float inputStrength = brain.GetNeuron(data.InputNeuron)->Activation;
		if (!dynamic_cast<ActionNeuron*>(brain.GetNeuron(data.OutputNeuron)))
		{
			inputStrength *= brain.GetNeuron(data.OutputNeuron)->Activation;
		}
		brain.mLinks[mLinksToShift.front()].Weight -= brain.mLearningSpeed * L * inputStrength;
		if (brain.mLinks[mLinksToShift.front()].Weight < brain.mMinimumLinkWeight)
		{
			brain.mLinks[mLinksToShift.front()].Weight = brain.mMinimumLinkWeight;
		}
		else if (brain.mLinks[mLinksToShift.front()].Weight > brain.mMaximumLinkWeight)
		{
			brain.mLinks[mLinksToShift.front()].Weight = brain.mMaximumLinkWeight;
		}
		mLinksToShift.pop();
	}
}
