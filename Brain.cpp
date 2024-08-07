#include "Brain.h"
#include "UtilityFunctions.h"
#include "Component.h"
#include "Agent.h"
#include "Actions.h"


#include <algorithm>
#include <queue>
#include <set>
#include <stack>
#include <map>
#include <cmath>
#include <fstream>

using namespace utility;

Brain::Brain(Agent* ControllingAgent, Agent* AgentToCopy)
{
	Brain* tempBrain = AgentToCopy->mAgentBrain.get();
	mControllingAgent = ControllingAgent;
	mNeuronIDCounter = tempBrain->GetNeuronIDCounter();
	mLinkIDCounter = tempBrain->GetLinkIDCounter();
	mLinks = tempBrain->GetLinks();

	/*mActionNeurons = tempBrain->mActionNeurons;
	mSensorNeurons = tempBrain->mSensorNeurons;*/
	mLayerIDCounter = tempBrain->mLayerIDCounter;
	mLearningSpeed = tempBrain->mLearningSpeed;

	mLayers = tempBrain->mLayers;
	mLayerOrder = tempBrain->mLayerOrder;

	for (auto& in : tempBrain->GetNeurons())
	{
		in.second->GenerateCopy(mNeurons[in.second->ID], in.second);
	}
}

void Brain::Update()
{
	if (!GetLayer(mLayerOrder[0])->NeuronIDs.empty() && !GetLayer(mLayerOrder.back())->NeuronIDs.empty())
	{
		mUpdatesSinceLastRandomAction++;
		mCurrentOverallActivationValue = 0.0f;
		CalculateOutputWeights();
		if(mUpdatesSinceLastRandomAction >= mUpdatesBetweenRandomAction)
		{ 
			ActivateRandomActionNeuron();
			mUpdatesSinceLastRandomAction = 0;
		}
		else {
			ActivateHighestActionNeuron();
		}
	}
}

void Brain::UpdateWeights()
{
	//Reward for the current action should be: diff in energy, and diff in how many nutrients are around the agent.
	mReward = mControllingAgent->GetActionReward();
	if (mLastTriggeredNeuron != nullptr && mControllingAgent->ShouldLearn())
	{
		//AdjustWeights();
		SimpleAdjustWeights();
	}
}

void Brain::CalculateOutputWeights()
{
	//Handle input neurons first
	for (int i = 0; i < GetLayer(mLayerOrder[0])->NeuronIDs.size(); i++)
	{
		SensorNeuron* sensor = static_cast<SensorNeuron*>(GetNeuron(GetLayer(mLayerOrder[0])->NeuronIDs[i]));
		sensor->CalculateWeightedSum(this);
	}

	for (int i = 1; i < mLayerOrder.size(); i++)
	{
		for (int j = 0; j < GetLayer(mLayerOrder[i])->NeuronIDs.size(); j++)
		{
			Neuron* neuron = GetNeuron(GetLayer(mLayerOrder[i])->NeuronIDs[j]);
			neuron->CalculateWeightedSum(this);
		}
	}
}

int Brain::ActivateHighestActionNeuron()
{
	float currentCount = 0.0f;
	float rand = utility::RandomNumber(0.0f, 1.0f);
	if (GetLayer(mLayerOrder.back())->NeuronIDs.size() > 0)
	{
		int i = 0;
		Neuron* checkNeuron = nullptr;
		while (i < GetLayer(mLayerOrder.back())->NeuronIDs.size() && currentCount < rand)
		{
			checkNeuron = mNeurons.find(GetLayer(mLayerOrder.back())->NeuronIDs[i])->second;
			currentCount += checkNeuron->Activation / mCurrentOverallActivationValue;
			i++;
		}
		if (checkNeuron != nullptr && checkNeuron->InputLinks.size() > 0 && checkNeuron->Activation > 0.0f)
		{
			ActionNeuron* actionNeuron = static_cast<ActionNeuron*>(checkNeuron);
			actionNeuron->TriggerAction(this);
			mLastTriggeredNeuron = actionNeuron;
		}
	}
	return -1;
}

int Brain::ActivateRandomActionNeuron()
{
	if (GetLayer(mLayerOrder.back())->NeuronIDs.size() > 0)
	{
		int rand = utility::RandomNumber(0, GetLayer(mLayerOrder.back())->NeuronIDs.size() - 1);
		int neuronID = GetLayer(mLayerOrder.back())->NeuronIDs[rand];
		ActionNeuron* actionNeuron = static_cast<ActionNeuron*>(GetNeuron(neuronID));
		actionNeuron->TriggerAction(this);
		mLastTriggeredNeuron = actionNeuron;
		return neuronID;
	}
	return -1;
}

void Brain::RandomizeLinks()
{
	//New refactored stuff

	for (int firstLayer = 0; firstLayer < mLayerOrder.size(); firstLayer++)
	{
		for (int inNeuron = 0; inNeuron < GetLayer(mLayerOrder[firstLayer])->NeuronIDs.size(); inNeuron++)
		{
			Neuron* currentInputNeuron = mNeurons.find(GetLayer(mLayerOrder[firstLayer])->NeuronIDs[inNeuron])->second;
			currentInputNeuron->Bias = RandomNumber(-1.0f, 1.0f) / 100.0f;
			for (int connectionLayer = firstLayer + 1; connectionLayer < mLayerOrder.size() && connectionLayer <= firstLayer + mMaxLinkDepth; connectionLayer++)
			{
				for (int outNeuron = 0; outNeuron < GetLayer(mLayerOrder[connectionLayer])->NeuronIDs.size(); outNeuron++)
				{
					Neuron* currentOutputNeuron = mNeurons.find(GetLayer(mLayerOrder[connectionLayer])->NeuronIDs[outNeuron])->second;
					if (RandomNumber(0.0f, 1.0f) <= mChanceToLink)
					{
						CreateLink(currentInputNeuron, currentOutputNeuron, RandomNumber(-0.5f, 1.0f));
					}
				}
			}
		}
	}
	//mLearningSpeed = utility::RandomNumber(0.1f, 0.3f);
	//mLinkRetention = utility::RandomNumber(1.0f, 10.0f);
}

void Brain::AddNewComponentNeurons(Component* ComponentToAdd)
{
	std::vector<float(*)(InputData, Agent*)> Inputs = ComponentToAdd->GetInputs();
	std::vector<INPUTTYPE> inputTypes = ComponentToAdd->GetInputTypes();
	for (int i = 0; i < Inputs.size(); i++)
	{
		int newNeuronID = AddNewSensorNeuron(Inputs[i], ComponentToAdd->GetInputData(), inputTypes[i]);
	}
	std::vector<void(*)(ActionData, Agent*)> Outputs = ComponentToAdd->GetActions();
	std::vector<ACTIONTYPE> actionTypes = ComponentToAdd->GetActionTypes();
	for (int i = 0; i < Outputs.size(); i++)
	{
		int newNeuronID = AddNewActionNeuron(Outputs[i], ComponentToAdd->GetActionData(), actionTypes[i]);
	}
}

int Brain::AddNewSensorNeuron(float (*input)(InputData, Agent*), InputData data, INPUTTYPE InputType)
{
	mNeuronIDCounter++;
	mNeurons[mNeuronIDCounter] = new SensorNeuron(input, data, mNeuronIDCounter, InputType);
	mNeurons[mNeuronIDCounter]->ContainingLayer = 0;
	GetLayer(mLayerOrder[0])->NeuronIDs.push_back(mNeuronIDCounter);
	return mNeuronIDCounter;
}

int Brain::AddNewActionNeuron(void (*action)(ActionData, Agent*), ActionData data, ACTIONTYPE ActionType)
{
	mNeuronIDCounter++;
	mNeurons[mNeuronIDCounter] = new ActionNeuron(action, data, mNeuronIDCounter, ActionType);
	mNeurons[mNeuronIDCounter]->ContainingLayer = 1;
	GetLayer(mLayerOrder.back())->NeuronIDs.push_back(mNeuronIDCounter);
	return mNeuronIDCounter;
}

int Brain::AddNewHiddenNeuron(int NetworkPos)
{
	if (NetworkPos < mLayerOrder.size() - 1 && NetworkPos > 0)
	{
		mNeuronIDCounter++;
		mNeurons[mNeuronIDCounter] = new Neuron(mNeuronIDCounter);
		mNeurons[mNeuronIDCounter]->ContainingLayer = mLayerOrder[NetworkPos];
		GetLayer(mLayerOrder[NetworkPos])->NeuronIDs.push_back(mNeuronIDCounter);
		return mNeuronIDCounter;
	}
	else
	{
		//Error adding neuron
		return -1;
	}
}
//If network number is greater than our current network depth, create new layers to fill in the gaps.
//If network number is already part of the network depth, we insert the new layer in front of the 
void Brain::AddNewHiddenLayer(int NetworkPos)
{
	auto it = mLayerOrder.begin();
	if (NetworkPos >= mLayerOrder.size())
	{
		int currentPos = mLayerOrder.size() - 1;
		while (NetworkPos >= mLayerOrder.size())
		{
			mLayerIDCounter++;
			it = mLayerOrder.begin();
			mLayerOrder.insert(std::next(it, currentPos), mLayerIDCounter);
			mLayers[mLayerIDCounter] = Layer();
			currentPos++;
		}
	}
	else
	{
		mLayerIDCounter++;
		mLayerOrder.insert(std::next(it, NetworkPos), mLayerIDCounter);
		mLayers[mLayerIDCounter] = Layer();
	}
}

void Brain::AdjustWeights()
{
	float currentReward = mReward;
	mObjectiveFunction = 0;
	//We need to: Simulate the future time steps that the agent would most likely take based on the new state
	//Add the reward gained based on these future steps, adjusted by the discount factor

	//Data that needs to be reset after predictions:
	//Agent position
	//Agent energy
	std::vector<ActionNeuron*> activatedNeurons(mFutureDepth + 1);
	std::vector<float> actionRewards(mFutureDepth + 1);
	std::vector<float> actionLossValues(mFutureDepth + 1);
	std::vector<float> probabilityValues(mFutureDepth + 1);
	activatedNeurons[0] = mLastTriggeredNeuron;
	probabilityValues[0] = mLastTriggeredNeuron->Activation / mCurrentOverallActivationValue;
	actionRewards[0] = mControllingAgent->GetActionReward();
	mObjectiveFunction = actionRewards[0] * probabilityValues[0];
	//currentReward += mControllingAgent->GetActionReward();
	utility::Vector2i initialPos = mControllingAgent->GetPosition();
	float initialEnergy = mControllingAgent->GetEnergy();
	float initialHealth = mControllingAgent->GetHealth();
	float initialOverallActivationValue = mCurrentOverallActivationValue;
	mSimulating = true;
	for (int i = 1; i < mFutureDepth + 1; i++)
	{
		mTrainingFutureDepth++;
		mCurrentOverallActivationValue = 0.0f;
		CalculateOutputWeights();
		ActivateHighestActionNeuron();

		activatedNeurons[i] = mLastTriggeredNeuron;
		probabilityValues[i] = mLastTriggeredNeuron->Activation / mCurrentOverallActivationValue;

		//Reward for the current action should be: diff in energy, and diff in how many nutrients are around the agent.
		//mReward = mControllingAgent->GetActionReward() * pow(mDiscountFactor, i);
		actionRewards[i] = mControllingAgent->GetActionReward() * pow(mDiscountFactor, i);
		//currentReward += mReward;
	}
	mObjectiveFunctionHistory.push_back(mObjectiveFunction);
	//Calculate discounted reward sum for each action
	for (int i = actionRewards.size() - 2; i >= 0; i--)
	{
		actionRewards[i] += actionRewards[i + 1];
	}

	//Reset values
	mTrainingFutureDepth = 0;
	mSimulating = false;
	mControllingAgent->SetPosition(initialPos);
	mControllingAgent->SetEnergy(initialEnergy);
	mControllingAgent->SetHealth(initialHealth);
	mCurrentOverallActivationValue = initialOverallActivationValue;
	for (int i = 0; i < mLayerOrder.size(); i++)
	{
		for (int j = 0; j < GetLayer(mLayerOrder[i])->NeuronIDs.size(); j++)
		{
			Neuron* neuron = GetNeuron(GetLayer(mLayerOrder[i])->NeuronIDs[j]);
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

	for (int i = 0; i < mFutureDepth + 1; i++)
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
				Link data = GetLinkData(currentNeuron->InputLinks[i]);
				mLinksToShift.push(data.ID);
				Neuron* inputNeuron = GetNeuron(data.InputNeuron);
				mNeuronsToCheck.push(inputNeuron);
			}
		}

		while (mLinksToShift.size() != 0)
		{
			Link data = GetLinkData(mLinksToShift.front());
			float inputStrength = GetNeuron(data.InputNeuron)->SimulationActivationHistory[i];
			float gradient = inputStrength * L * mLearningSpeed;
			mLinks[mLinksToShift.front()].Weight -= gradient;
			if (mLinks[mLinksToShift.front()].Weight < mMinimumLinkWeight)
			{
				mLinks[mLinksToShift.front()].Weight = mMinimumLinkWeight;
			}
			else if (mLinks[mLinksToShift.front()].Weight > mMaximumLinkWeight)
			{
				mLinks[mLinksToShift.front()].Weight = mMaximumLinkWeight;
			}
			mLinksToShift.pop();
		}
	}
}

void Brain::SimpleAdjustWeights()
{
	float currentReward = mReward;
	float probability = mLastTriggeredNeuron->Activation / mCurrentOverallActivationValue;
	//We need to: Simulate the future time steps that the agent would most likely take based on the new state
	//Add the reward gained based on these future steps, adjusted by the discount factor

	//Data that needs to be reset after predictions:
	//Agent position
	//Agent energy
	utility::Vector2i initialPos = mControllingAgent->GetPosition();
	float initialEnergy = mControllingAgent->GetEnergy();
	float initialOverallActivationValue = mCurrentOverallActivationValue;
	mSimulating = true;
	for (int i = 1; i < mFutureDepth + 1; i++)
	{
		mCurrentOverallActivationValue = 0.0f;
		CalculateOutputWeights();
		ActivateHighestActionNeuron();
		//Reward for the current action should be: diff in energy, and diff in how many nutrients are around the agent.
		mReward = mControllingAgent->GetActionReward() * pow(mDiscountFactor, i);
		currentReward += mReward;
	}
	mValue = currentReward;
	mSimulating = false;
	mControllingAgent->SetPosition(initialPos);
	mControllingAgent->SetEnergy(initialEnergy);
	mCurrentOverallActivationValue = initialOverallActivationValue;

	float L = 0;
	L = mValue * log(probability);

	//TODO: optimize policy using L
	//At least this time it's not *worse* than complete random, but it's not quite right.
	Neuron* triggeredNeuron = mLastTriggeredNeuron;
	std::queue<Neuron*> mNeuronsToCheck({ triggeredNeuron });
	std::queue<int> mLinksToShift;
	while (mNeuronsToCheck.size() != 0)
	{
		Neuron* currentNeuron = mNeuronsToCheck.front();
		currentNeuron->Activation = currentNeuron->InitialEpisodeActivation;
		mNeuronsToCheck.pop();
		for (int i = 0; i < currentNeuron->InputLinks.size(); i++)
		{
			Link data = GetLinkData(currentNeuron->InputLinks[i]);
			mLinksToShift.push(data.ID);
			Neuron* inputNeuron = GetNeuron(data.InputNeuron);
			mNeuronsToCheck.push(inputNeuron);
		}
	}
	while (mLinksToShift.size() != 0)
	{
		Link data = GetLinkData(mLinksToShift.front());
		float inputStrength = GetNeuron(data.InputNeuron)->Activation;
		if (!dynamic_cast<ActionNeuron*>(GetNeuron(data.OutputNeuron)))
		{
			inputStrength *= GetNeuron(data.OutputNeuron)->Activation;
		}
		mLinks[mLinksToShift.front()].Weight -= mLearningSpeed * L * inputStrength;
		if (mLinks[mLinksToShift.front()].Weight < mMinimumLinkWeight)
		{
			mLinks[mLinksToShift.front()].Weight = mMinimumLinkWeight;
		}
		else if (mLinks[mLinksToShift.front()].Weight > mMaximumLinkWeight)
		{
			mLinks[mLinksToShift.front()].Weight = mMaximumLinkWeight;
		}
		mLinksToShift.pop();
	}
}

void Brain::CreateLink(Neuron* inputNeuron, Neuron* outputNeuron, float weight)
{
	long newLinkValue = CreateLinkValue(inputNeuron->ID, outputNeuron->ID);
	Link newLink = Link(inputNeuron->ID, outputNeuron->ID, weight, newLinkValue);
	mLinks[newLinkValue] = newLink;

	inputNeuron->OutputLinks.push_back(newLinkValue);
	outputNeuron->InputLinks.push_back(newLinkValue);

	mLinkIDCounter++;
}

void Brain::CreateLink(int inputNeuronID, int outputNeuronID, float weight)
{
	Neuron* inputNeuron = GetNeuron(inputNeuronID);
	Neuron* outputNeuron = GetNeuron(outputNeuronID);
	CreateLink(inputNeuron, outputNeuron, weight);
}

void Brain::DeleteLink(Link linkToDelete)
{
	Neuron* inputNeuron = mNeurons[linkToDelete.InputNeuron];
	Neuron* outputNeuron = mNeurons[linkToDelete.OutputNeuron];
	auto it = std::remove(inputNeuron->OutputLinks.begin(), inputNeuron->OutputLinks.end(), linkToDelete.ID);
	inputNeuron->OutputLinks.erase(it, inputNeuron->OutputLinks.end());

	it = std::remove(outputNeuron->InputLinks.begin(), outputNeuron->InputLinks.end(), linkToDelete.ID);
	outputNeuron->InputLinks.erase(it, outputNeuron->InputLinks.end());

	mLinks.erase(linkToDelete.ID);
}

void Brain::DeleteLink(long linkToDelete)
{
	DeleteLink(mLinks.at(linkToDelete));
}

bool Brain::DoesLinkExist(int inputNeuron, int outputNeuron)
{
	long linkVal = CreateLinkValue(inputNeuron, outputNeuron);
	Link check = mLinks[linkVal];
	if (check.ID != -1)
	{
		return true;
	}
	else {
		return false;
	}
}
//Decode a long value into an input and an output value
void Brain::DecodeLinkValue(long linkValue, int& inputID, int& outputID)
{
	int w = floor((sqrt(8 * linkValue + 1) - 1) / 2);
	int t = (w * w + w) / 2;
	outputID = linkValue - t;
	inputID = w - outputID;
}
//Create a unique value based on an input and an output int
long Brain::CreateLinkValue(int input, int output)
{
	int a = input;
	int b = output;
	long linkVal = ((a + b) * (a + b + 1)) / 2 + b;
	return linkVal;
}

Neuron* Brain::GetNeuron(int ID)
{
	return mNeurons.at(ID);
}

const Link Brain::GetLinkData(int ID)
{
	return mLinks.at(ID);
}

void Brain::StoreLinkShiftData(int ID, float Shift)
{
	mLinks.at(ID).WeightHistory += Shift;
	/*if (mLinks.at(ID).Weight > 30.0f)
	{
		mLinks.at(ID).Weight = 30.0f;
	}
	else if (mLinks.at(ID).Weight < -30.0f)
	{
		mLinks.at(ID).Weight = -30.0f;
	}*/
}

void Brain::ShiftLinkWeight(int ID, float amount)
{
	mLinks.at(ID).Weight += amount;
}

void Brain::CreateRandomLink()
{
	int layerToStartOn = utility::RandomNumber(0, mLayerOrder.size() - 2);
	int layerToTarget = utility::RandomNumber(layerToStartOn + 1, mLayerOrder.size() - 1);

	Layer startLayer = *GetLayer(mLayerOrder[layerToStartOn]);
	Layer targetLayer = *GetLayer(mLayerOrder[layerToTarget]);

	if (startLayer.NeuronIDs.size() > 0 && targetLayer.NeuronIDs.size() > 0)
	{
		int neuronToStartOn = utility::RandomNumber(0, startLayer.NeuronIDs.size() - 1);
		int startNeuronID = startLayer.NeuronIDs[neuronToStartOn];

		int neuronToTarget = utility::RandomNumber(0, targetLayer.NeuronIDs.size() - 1);
		int targetNeuronID = targetLayer.NeuronIDs[neuronToTarget];

		if (!DoesLinkExist(startNeuronID, targetNeuronID))
		{
			CreateLink(startNeuronID, targetNeuronID, utility::RandomNumber(0.0f, 1.0f));
		}
	}
}

void Brain::DestroyRandomLink()
{
	int randLayerNum = utility::RandomNumber(0, mLayerOrder.size() - 1);
	Layer randLayer = *GetLayer(mLayerOrder[randLayerNum]);
	if (randLayer.NeuronIDs.size() > 0)
	{
		int randNeuronNum = utility::RandomNumber(0, randLayer.NeuronIDs.size() - 1);
		Neuron* randNeuron = GetNeuron(randLayer.NeuronIDs[randNeuronNum]);
		if (randNeuron->OutputLinks.size() > 0)
		{
			int randLinkNum = utility::RandomNumber(0, randNeuron->OutputLinks.size() - 1);
			DeleteLink(randNeuron->OutputLinks[randLinkNum]);
		}
	}
}

void Brain::SplitRandomLink()
{
	//To split a link we need to delete the old link, insert a new neuron and then create two additional links.
	int randLayerNum = utility::RandomNumber(0, mLayerOrder.size() - 1);
	Layer randLayer = *GetLayer(mLayerOrder[randLayerNum]);
	if (randLayer.NeuronIDs.size() > 0)
	{
		int randNeuronNum = utility::RandomNumber(0, randLayer.NeuronIDs.size() - 1);
		Neuron* randNeuron = GetNeuron(randLayer.NeuronIDs[randNeuronNum]);
		if (randNeuron->OutputLinks.size() > 0)
		{
			int newNeuron = -1;
			int randLinkNum = utility::RandomNumber(0, randNeuron->OutputLinks.size() - 1);
			Link randLink = mLinks[randNeuron->OutputLinks[randLinkNum]];

			Neuron* outputNeuron = mNeurons[randLink.OutputNeuron];
			int outputLayerID = outputNeuron->ContainingLayer;
			int outputLayerPosition = -1;
			for (int i = randLayerNum + 1; i < mLayerOrder.size(); i++)
			{
				if (mLayerOrder[i] == outputLayerID)
				{
					outputLayerPosition = i;
				}
			}

			if (outputLayerPosition != -1)
			{
				if (outputLayerPosition - randLayerNum == 1)
				{
					//We need to add a hidden 
					AddNewHiddenLayer(outputLayerPosition);
					newNeuron = AddNewHiddenNeuron(outputLayerPosition);
				}
				else
				{
					newNeuron = AddNewHiddenNeuron(randLayerNum + 1);
				}

				CreateLink(randNeuron->ID, newNeuron, 0.5f);
				CreateLink(newNeuron, outputNeuron->ID, 0.5f);
				DeleteLink(randNeuron->OutputLinks[randLinkNum]);
			}
			else
			{
				//Error finding layer
			}
		}
	}
}

void Brain::SaveBrain(std::string FileName)
{
	constexpr auto const CISMODE = cista::mode::WITH_VERSION |
		cista::mode::WITH_INTEGRITY |
		cista::mode::DEEP_CHECK;

	/*for (auto& it : mTestObjects)
	{
		testObjectList[it.first] = *it.second;
	}

	auto testobj = cista::serialize<CISMODE>(testObjectList);
	data::hash_map<int, TestObject> testoo = *cista::deserialize<data::hash_map<int, TestObject>, CISMODE>(testobj);*/
	
	/*for (auto& it : testoo)
	{
		TestObject* test = &it.second;
		if (test == nullptr)
		{
			bool yeet = false;
		}
	}*/

	for (auto& it : mNeurons)
	{
		NeuronData savedNeuronData = NeuronData();
		savedNeuronData.Bias = it.second->Bias;
		savedNeuronData.InputLinks = it.second->InputLinks;
		savedNeuronData.OutputLinks = it.second->OutputLinks;
		savedNeuronData.ContainingLayer = it.second->ContainingLayer;
		savedNeuronData.ID = it.second->ID;
		
		if (ActionNeuron* neuron = dynamic_cast<ActionNeuron*>(it.second))
		{
			savedNeuronData.ActionType = neuron->actionType;
			savedNeuronData.actionNeuron = true;
			switch (neuron->actionType)
			{
			case(EAT):
			{
				auto& data = std::get<MouthData>(neuron->NeuronActionData);
				savedNeuronData.AmountToEat = data.AmountToEat;
				break;
			}
			case(ATTACK):
			{
				auto& data = std::get<AttackData>(neuron->NeuronActionData);
				savedNeuronData.DamageToDeal = data.DamageToDeal;
				savedNeuronData.Efficiency = data.Efficiency;
				break;
			}
			case(MOVEFORWARD):
			{
				auto& data = std::get<MovementData>(neuron->NeuronActionData);
				savedNeuronData.ClimbingEfficiency = data.ClimbingEfficiency;
			}
			}
		}
		else if (SensorNeuron* neuron = dynamic_cast<SensorNeuron*>(it.second))
		{
			savedNeuronData.InputType = neuron->inputType;
			savedNeuronData.sensorNeuron = true;
			switch (neuron->inputType)
			{
			case(NUTRIENTSENSE || AGENTSENSE || SOLIDSENSE || HEIGHTSENSE || LAVASENSE || POISONSENSE):
				auto& data = std::get<EyeData>(neuron->NeuronInputData);
				savedNeuronData.ObservationPosX = data.ObservationPosX;
				savedNeuronData.ObservationPosY = data.ObservationPosY;
				break;
			}
		}

		serialize_Neurons[it.first] = savedNeuronData;
		//Neuron* test = &serialize_Neurons[it.first];
	}
	for (auto& it : mLinks)
	{
		serialize_Links[it.first] = it.second;
	}
	for (auto& it : mLayers)
	{
		serialize_Layers[it.first] = it.second;
	}

	auto savedBrain = cista::serialize<CISMODE, Brain>(*this);
	//Brain* testBrain = cista::deserialize<Brain, CISMODE>(savedBrain);
	//SetupBrainFromLoadedBrain(testBrain);
	//dataTest = cista::serialize<CISMODE, Brain>(*this);
	std::string path = "Brains/" + FileName + ".txt";
	std::ofstream file(path, std::ios::out | std::ios::binary);
	file.write(reinterpret_cast<const char*>(savedBrain.data()), savedBrain.size());
	file.close();
}

void Brain::SetupBrainFromLoadedBrain(Brain* LoadedBrain)
{
	for (auto& it : LoadedBrain->serialize_Links)
	{
		mLinks[it.first] = it.second;
	}
	for (auto& it : LoadedBrain->serialize_Neurons)
	{
		if (it.second.actionNeuron)
		{
			mNeurons[it.first] = new ActionNeuron(it.second);
		}
		else if (it.second.sensorNeuron)
		{
			mNeurons[it.first] = new SensorNeuron(it.second);
		}
		else
		{
			mNeurons[it.first] = new Neuron(it.second);
		}
	}
	for (auto& it : LoadedBrain->serialize_Layers)
	{
		mLayers[it.first] = it.second;
	}
	mLayerIDCounter = LoadedBrain->mLayerIDCounter;
	mLearningSpeed = LoadedBrain->mLearningSpeed;
	mLinkIDCounter = LoadedBrain->mLinkIDCounter;
	mNeuronIDCounter = LoadedBrain->mNeuronIDCounter;
}