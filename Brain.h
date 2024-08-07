#pragma once

#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <cista.h>
#include "ComponentAction.h"
#include "ComponentInput.h"
#include "Globals.h"
#include "Neurons.h"
#include "NeuronData.h"

namespace data = cista::offset;

class Agent;
class Component;
class Actions;

struct Layer
{
	data::vector<int> NeuronIDs;
	auto cista_members()
	{
		return std::tie(
			NeuronIDs
		);
	}
};
//
//template <typename Ctx>
//inline void serialize(Ctx& context, Layer const* el,
//	cista::offset_t const offset) {
//	using cista::serialize;
//	serialize(context, &el->NeuronIDs, offset + offsetof(Layer, NeuronIDs));
//}
//
//template <typename Ctx>
//inline void deserialize(Ctx const& context, Layer* el) {
//	using cista::deserialize;
//	deserialize(context, &el->NeuronIDs);
//}

struct Link
{
	Link()
	{

	}

	Link(int inputNeuron, int outputNeuron, float weight, int id)
	{
		InputNeuron = inputNeuron;
		OutputNeuron = outputNeuron;
		Weight = weight;
		ID = id;
	}

	int ID = -1;

	int InputNeuron = -1;
	int OutputNeuron = -1;

	float Weight = 0.0f;

	float WeightHistory = 0.0f;

	auto cista_members()
	{
		return std::tie(
			ID,
			InputNeuron,
			OutputNeuron,
			Weight
		);
	}
};


//template <typename Ctx>
//inline void serialize(Ctx& context, Link const* el,
//	cista::offset_t const offset) {
//	using cista::serialize;
//	serialize(context, &el->ID, offset + offsetof(Link, ID));
//	serialize(context, &el->InputNeuron, offset + offsetof(Link, InputNeuron));
//	serialize(context, &el->OutputNeuron, offset + offsetof(Link, OutputNeuron));
//	serialize(context, &el->Weight, offset + offsetof(Link, Weight));
//	// call serialize on all members!
//	// always keep this up-to-date as you add member variables!
//}
//
//template <typename Ctx>
//inline void deserialize(Ctx const& context, Link* el) {
//	using cista::deserialize;
//	deserialize(context, &el->ID);
//	deserialize(context, &el->InputNeuron);
//	deserialize(context, &el->OutputNeuron);
//	deserialize(context, &el->Weight);
//}


//When calculating output:
//We go through each neuron in the second layer
//We need to know which neurons are linked to the neuron
//So neurons need to know their input links
//Links need to know their input neuron

//When getting out input and output neurons:
//We need an array or vector of floats from 0 to 1 for our inputs
//We need an array or vector of floats from 0 to 1 for our outputs
//Get index of output that has the highest value and trigger?
//Or store a required activation value within actions which needs to be met to trigger?

class Brain
{
	friend class Brain;
	
	std::unordered_map<int, Neuron*> mNeurons;
	std::unordered_map<long, Link> mLinks;
	std::unordered_map<int, Layer> mLayers;

	float mMinimumLinkWeight = -10.0f;
	float mMaximumLinkWeight = 10.0f;

	float mChanceToLink = 1.0f;

	int mMaxLinkDepth = 1;

	Agent* mControllingAgent = nullptr;

	ActionNeuron* mLastTriggeredNeuron = nullptr;

	int mUpdatesBetweenShifts = 0;
	int mNumUpdatesSinceLastShift = 0;

	int mUpdatesSinceLastRandomAction = 0;
	int mUpdatesBetweenRandomAction = 10;

	//Learning values
	//Reward (r) is the value given to how good the previous action was.
	float mReward = 0.0f;
	//Value (V) is the expected long term return with discount of the current state.
	//We get the mValue by adding together the rewards gained from future states, modified by discount factor.
	float mValue = 0.0f;
	
	

public:

	int mMaxNumOfInitComponents = 15;
	int mMaxEyeViewDist = 3;
	int mMaxHiddenNeurons = 10;
	int mMaxHiddenLayers = 0;

	bool mSimulating = false;
	int mTrainingFutureDepth = 0;

	//Discount factor is multiplied by future rewards to make future rewards worth less.
	float mDiscountFactor = 0.8f;
	//How deep into the future we check for our reward
	int mFutureDepth = 10;

	//Data required for serialization
	//========================================
	data::hash_map<int, NeuronData> serialize_Neurons;
	data::hash_map<long, Link> serialize_Links;
	data::hash_map<int, Layer> serialize_Layers;
	//.front = input layer
	//.back = output layer
	data::vector<int> mLayerOrder = { 0, 1 };
	int mNeuronIDCounter = 0;
	int mLinkIDCounter = 0;
	int mLayerIDCounter = 1;

	float mLearningSpeed = 0.05f;
	float mObjectiveFunction = 0.0f;
	std::vector<float> mObjectiveFunctionHistory;
	//========================================

	Brain()
	{
		mControllingAgent = nullptr;
		mLastTriggeredNeuron = nullptr;
	}

	Brain(Agent* ControllingAgent)
	{
		mControllingAgent = ControllingAgent;
	}

	Brain(Agent* ControllingAgent, Agent* AgentToCopy);

	~Brain()
	{

	}

	int GetNeuronIDCounter() { return mNeuronIDCounter; }
	int GetLinkIDCounter() { return mLinkIDCounter; }

	std::unordered_map<long, Link> GetLinks() { return mLinks; };

	Agent* GetControllingAgent()
	{
		return mControllingAgent;
	}
	void SetControllingAgent(Agent* controllingAgent)
	{
		mControllingAgent = controllingAgent;
	}

	void Update();
	void UpdateWeights();

	//void AssignComponentSensorsAndActions(Component* ComponentToCopy);
	void AddNewComponentNeurons(Component* ComponentToAdd);
	int AddNewSensorNeuron(float (*input)(InputData, Agent*), InputData data, INPUTTYPE inputType);
	int AddNewActionNeuron(void (*action)(ActionData, Agent*), ActionData data, ACTIONTYPE actionType);
	int AddNewHiddenNeuron(int NetworkPos);
	//When adding a new hidden layer at a given depth, we need to create new layers to fill in the gaps
	void AddNewHiddenLayer(int NetworkPos);

	void AdjustWeights();
	void SimpleAdjustWeights();

	void CalculateOutputWeights();
	int ActivateHighestActionNeuron();
	int ActivateRandomActionNeuron();

	void SetConnectionChance(float val) { mChanceToLink = val; }
	void SetRandomActionUpdateCount(int val) { mUpdatesBetweenRandomAction = val; };
	void RandomizeLinks();

	void CreateLink(Neuron* inputNeuron, Neuron* outputNeuron, float weight);
	void CreateLink(int inputNeuronID, int outputNeuronID, float weight);
	void DeleteLink(Link linkToDelete);
	void DeleteLink(long linkToDelete);
	bool DoesLinkExist(int inputNeuron, int outputNeuron);

	long CreateLinkValue(int input, int output);
	void DecodeLinkValue(long linkValue, int& inputID, int& outputID);

	Neuron* GetNeuron(int ID);
	const std::unordered_map<int, Neuron*> GetNeurons()
	{
		return mNeurons;
	}

	const Link GetLinkData(int ID);
	void StoreLinkShiftData(int ID, float shift);
	void ShiftLinkWeight(int ID, float amount);

	

	Layer* GetLayer(int layerID)
	{
		return &mLayers[layerID];
	}
	const std::unordered_map<int, Layer> GetLayers() const
	{
		return mLayers;
	}
	const data::vector<int> GetLayerOrder() const
	{
		return mLayerOrder;
	}

	float GetLearningRate()
	{
		return mLearningSpeed;
	}

	float mCurrentOverallActivationValue = 0.0f;

	//Mutations

	void CreateRandomLink();
	void DestroyRandomLink();
	void SplitRandomLink();

	void SaveBrain(std::string FileName);
	void SetupBrainFromLoadedBrain(Brain* LoadedBrain);
	auto cista_members() 
	{
		return std::tie(
			serialize_Links,
			serialize_Layers,
			serialize_Neurons,
			mLayerIDCounter,
			mLearningSpeed,
			mLinkIDCounter,
			mNeuronIDCounter
		);
	}
};

//template <typename Ctx>
//inline void serialize(Ctx& context, Brain const* el,
//	cista::offset_t const offset) {
//	using cista::serialize;
//	
//	serialize(context, &el->serialize_Links, offset + offsetof(Brain, serialize_Links));
//	serialize(context, &el->serialize_Layers,	offset + offsetof(Brain, serialize_Layers));
//	serialize(context, &el->serialize_Neurons,	offset + offsetof(Brain, serialize_Neurons));
//	serialize(context, &el->mActionIDCounter,	offset + offsetof(Brain, mActionIDCounter));
//	serialize(context, &el->mInputIDCounter,	offset + offsetof(Brain, mInputIDCounter));
//	serialize(context, &el->mLayerIDCounter,	offset + offsetof(Brain, mLayerIDCounter));
//	serialize(context, &el->mLearningSpeed,		offset + offsetof(Brain, mLearningSpeed));
//	serialize(context, &el->mLinkIDCounter,		offset + offsetof(Brain, mLinkIDCounter));
//	serialize(context, &el->mNeuronIDCounter,	offset + offsetof(Brain, mNeuronIDCounter));
//	// call serialize on all members!
//	// always keep this up-to-date as you add member variables!
//}
//
//template <typename Ctx>
//inline void deserialize(Ctx const& context, Brain* el) {
//	using cista::deserialize;
//	deserialize(context, &el->serialize_Links);
//	deserialize(context, &el->serialize_Layers);
//	deserialize(context, &el->serialize_Neurons);
//	deserialize(context, &el->mActionIDCounter);
//	deserialize(context, &el->mInputIDCounter);
//	deserialize(context, &el->mLayerIDCounter);
//	deserialize(context, &el->mLearningSpeed);
//	deserialize(context, &el->mLinkIDCounter);
//	deserialize(context, &el->mNeuronIDCounter);
//	// call deserialize on all members!
//	// always keep this up-to-date as you add member variables!
//}

