#include "FullAlgorithm.h"
#include "Brain.h"
#include "Agent.h"

void FullAlgorithm::AdjustWeights(Brain& brain)
{
	float currentReward = brain.mReward;
	brain.mObjectiveFunction = 0;
	//We need to: Simulate the future time steps that the agent would most likely take based on the new state
	//Add the reward gained based on these future steps, adjusted by the discount factor

	//Data that needs to be reset after predictions:
	//Agent position
	//Agent energy
	std::vector<ActionNeuron*> activatedNeurons(brain.mFutureDepth + 1);
	std::vector<float> actionRewards(brain.mFutureDepth + 1);
	std::vector<float> actionLossValues(brain.mFutureDepth + 1);
	std::vector<float> probabilityValues(brain.mFutureDepth + 1);
	activatedNeurons[0] = brain.mLastTriggeredNeuron;
	probabilityValues[0] = brain.mLastTriggeredNeuron->Activation / brain.mCurrentOverallActivationValue;
	actionRewards[0] = brain.mControllingAgent->GetActionReward();
	brain.mObjectiveFunction = actionRewards[0] * probabilityValues[0];
	//currentReward += mControllingAgent->GetActionReward();
	utility::Vector2i initialPos = brain.mControllingAgent->GetPosition();
	float initialEnergy = brain.mControllingAgent->GetEnergy();
	float initialHealth = brain.mControllingAgent->GetHealth();
	float initialOverallActivationValue = brain.mCurrentOverallActivationValue;
	brain.mSimulating = true;
	for (int i = 1; i < brain.mFutureDepth + 1; i++)
	{
		brain.mTrainingFutureDepth++;
		brain.mCurrentOverallActivationValue = 0.0f;
		brain.CalculateOutputWeights();
		brain.ActivateHighestActionNeuron();

		activatedNeurons[i] = brain.mLastTriggeredNeuron;
		probabilityValues[i] = brain.mLastTriggeredNeuron->Activation / brain.mCurrentOverallActivationValue;

		//Reward for the current action should be: diff in energy, and diff in how many nutrients are around the agent.
		//mReward = mControllingAgent->GetActionReward() * pow(mDiscountFactor, i);
		actionRewards[i] = brain.mControllingAgent->GetActionReward() * pow(brain.mDiscountFactor, i);
		//currentReward += mReward;
	}
	brain.mObjectiveFunctionHistory.push_back(brain.mObjectiveFunction);
	//Calculate discounted reward sum for each action
	for (int i = actionRewards.size() - 2; i >= 0; i--)
	{
		actionRewards[i] += actionRewards[i + 1];
	}

	//Reset values
	brain.mTrainingFutureDepth = 0;
	brain.mSimulating = false;
	brain.mControllingAgent->SetPosition(initialPos);
	brain.mControllingAgent->SetEnergy(initialEnergy);
	brain.mControllingAgent->SetHealth(initialHealth);
	brain.mCurrentOverallActivationValue = initialOverallActivationValue;
	for (int i = 0; i < brain.mLayerOrder.size(); i++)
	{
		for (int j = 0; j < brain.GetLayer(brain.mLayerOrder[i])->NeuronIDs.size(); j++)
		{
			Neuron* neuron = brain.GetNeuron(brain.GetLayer(brain.mLayerOrder[i])->NeuronIDs[j]);
			if (neuron->HasBeenSimulated)
			{
				neuron->Activation = neuron->SimulationActivationHistory[0];
				neuron->HasBeenSimulated = false;
			}
		}
	}
	//Probability of the given 
	Neuron* triggeredNeuron = nullptr;
	std::queue<Neuron*> mNeuronsToCheck;
	std::queue<int> mLinksToShift;

	for (int i = 0; i < brain.mFutureDepth + 1; i++)
	{
		float L = actionRewards[i] * log(probabilityValues[i]);
		Neuron* triggeredNeuron = activatedNeurons[i];
		mNeuronsToCheck.push(triggeredNeuron);
		while (mNeuronsToCheck.size() != 0)
		{
			Neuron* currentNeuron = mNeuronsToCheck.front();
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
			float inputStrength = brain.GetNeuron(data.InputNeuron)->SimulationActivationHistory[i];
			float gradient = inputStrength * L * brain.mLearningSpeed;
			brain.mLinks[mLinksToShift.front()].Weight -= gradient;
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
}
