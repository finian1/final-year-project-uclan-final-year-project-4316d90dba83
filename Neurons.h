#pragma once

#include <vector>
#include <cista.h>
#include "ComponentAction.h"
#include "ComponentInput.h"
#include "Globals.h"
#include "NeuronData.h"

namespace data = cista::offset;

class Agent;
class Brain;
struct Link;

struct NeuronData
{
	float Bias = 0.0f;
	data::vector<int> InputLinks;
	data::vector<int> OutputLinks;
	int ContainingLayer = -1;
	int ID = -1;

	bool actionNeuron = false;
	ACTIONTYPE ActionType = ACTIONTYPE::NOACTION;
	float AmountToEat = 0.0f;
	float DamageToDeal = 0.0f;
	float Efficiency = 0.0f;
	float ClimbingEfficiency = 0.0f;
	//void (*NeuronAction)(ActionData, Agent*) {};
	//ActionData NeuronActionData = ActionData{};

	bool sensorNeuron = false;
	INPUTTYPE InputType = INPUTTYPE::NOINPUT;
	//Possible Input Data
	int ObservationPosX = 0;
	int ObservationPosY = 0;

	//float (*NeuronInput)(InputData, Agent*) {};
	//InputData NeuronInputData = InputData{};


	auto cista_members()
	{
		return std::tie(
			Bias,
			InputLinks,
			OutputLinks,
			ContainingLayer,
			ID,
			AmountToEat,
			DamageToDeal,
			Efficiency,
			ClimbingEfficiency,
			ObservationPosX,
			ObservationPosY
		);
	}
};

struct Neuron
{
	Neuron()
	{

	}

	Neuron(NeuronData data)
	{
		ID = data.ID;
		Bias = data.Bias;
		InputLinks = data.InputLinks;
		OutputLinks = data.OutputLinks;
		ContainingLayer = data.ContainingLayer;
	}

	Neuron(Neuron* neuronToCopy)
	{
		ID = neuronToCopy->ID;
		Bias = neuronToCopy->Bias;
		InputLinks = neuronToCopy->InputLinks;
		OutputLinks = neuronToCopy->OutputLinks;
		ContainingLayer = neuronToCopy->ContainingLayer;
	}

	Neuron(int NeuronID)
	{
		ID = NeuronID;
	}
	float Activation = 0.5f;
	//Temp future activation is used to get the state of future time steps without overriding current activation.
	float TempFutureActivation = 0.0f;
	float Bias = 0.0f;

	//Delta used by backpropagation
	float LossDelta = 0.0f;

	data::vector<int> InputLinks;
	data::vector<int> OutputLinks;

	//For network display
	float disPosX = -1.0f;
	float disPosY = -1.0f;

	int ContainingLayer = -1;
	//Brain* ControllingBrain = nullptr;

	int ID = -1;

	std::unordered_map<int, float> SimulationActivationHistory;
	float InitialEpisodeActivation;
	bool HasBeenSimulated = false;

	virtual void CalculateWeightedSum(Brain* brain);

	virtual void GenerateCopy(Neuron*& copy, Neuron* neuronToCopy)
	{
		copy = new Neuron(neuronToCopy);
	}
};

struct ActionNeuron : public Neuron
{
	ActionNeuron(NeuronData data)
	{
		ID = data.ID;
		Bias = data.Bias;
		InputLinks = data.InputLinks;
		OutputLinks = data.OutputLinks;
		ContainingLayer = data.ContainingLayer;
		NeuronAction = gActionMap.at(data.ActionType);
		switch (data.ActionType)
		{
		case(ACTIONTYPE::ATTACK):
			NeuronActionData = AttackData{ data.DamageToDeal, data.Efficiency };
			break;
		case(ACTIONTYPE::EAT):
			NeuronActionData = MouthData{ data.AmountToEat };
			break;
		case(ACTIONTYPE::MOVEFORWARD):
			NeuronActionData = MovementData{ data.ClimbingEfficiency };
		default:
			break;
		}
	}

	ActionNeuron(ActionNeuron* neuronToCopy)
		: Neuron(neuronToCopy)
	{
		NeuronAction = neuronToCopy->NeuronAction;
		NeuronActionData = neuronToCopy->NeuronActionData;
		actionType = neuronToCopy->actionType;
	}

	ActionNeuron(void (*action)(ActionData, Agent*), ActionData data, int NeuronID, ACTIONTYPE ActionType)
		: Neuron(NeuronID)
	{
		NeuronAction = action;
		NeuronActionData = data;
		actionType = ActionType;
	}

	ACTIONTYPE actionType;
	void (*NeuronAction)(ActionData, Agent*) {};
	ActionData NeuronActionData = ActionData{};

	std::unordered_map<int, float> SimulationProbabilityHistory;

	void CalculateWeightedSum(Brain* brain) override;

	void TriggerAction(Brain* brain);

	void GenerateCopy(Neuron*& copy, Neuron* neuronToCopy) override
	{
		copy = new ActionNeuron(static_cast<ActionNeuron*>(neuronToCopy));
	}
};

struct SensorNeuron : public Neuron
{
	SensorNeuron(NeuronData data)
	{
		ID = data.ID;
		Bias = data.Bias;
		InputLinks = data.InputLinks;
		OutputLinks = data.OutputLinks;
		ContainingLayer = data.ContainingLayer;
		NeuronInput = gInputMap.at(data.InputType);
		switch (data.InputType)
		{
		case(INPUTTYPE::AGENTSENSE):
			NeuronInputData = EyeData{ data.ObservationPosX, data.ObservationPosY };
			break;
		case(INPUTTYPE::NUTRIENTSENSE):
			NeuronInputData = EyeData{ data.ObservationPosX, data.ObservationPosY };
			break;
		case(INPUTTYPE::SOLIDSENSE):
			NeuronInputData = EyeData{ data.ObservationPosX, data.ObservationPosY };
			break;
		case(INPUTTYPE::HEIGHTSENSE):
			NeuronInputData = EyeData{ data.ObservationPosX, data.ObservationPosY };
			break;
		case(INPUTTYPE::LAVASENSE):
			NeuronInputData = EyeData{ data.ObservationPosX, data.ObservationPosY };
			break;
		case(INPUTTYPE::POISONSENSE):
			NeuronInputData = EyeData{ data.ObservationPosX, data.ObservationPosY };
			break;
		}
	}

	SensorNeuron(SensorNeuron* neuronToCopy)
		: Neuron(neuronToCopy)
	{
		NeuronInput = neuronToCopy->NeuronInput;
		NeuronInputData = neuronToCopy->NeuronInputData;
		inputType = neuronToCopy->inputType;
	}

	SensorNeuron(float (*input)(InputData, Agent*), InputData data, int NeuronID, INPUTTYPE InputType) :
		Neuron(NeuronID)
	{
		NeuronInput = input;
		NeuronInputData = data;
		inputType = InputType;
	}

	INPUTTYPE inputType;
	float (*NeuronInput)(InputData, Agent*) {};
	InputData NeuronInputData = InputData{};

	float TriggerInput(Brain* brain);

	void CalculateWeightedSum(Brain* brain) override;

	void GenerateCopy(Neuron*& copy, Neuron* neuronToCopy) override
	{
		copy = new SensorNeuron(static_cast<SensorNeuron*>(neuronToCopy));
	}
};

struct HiddenNeuron : public Neuron
{
	HiddenNeuron(HiddenNeuron* neuronToCopy) : Neuron(neuronToCopy)
	{
	}
};



//struct TestObject
//{
//	TestObject()
//	{
//
//	}
//	TestObject(int a)
//	{
//		A = a;
//	}
//
//	int A = 20;
//	data::vector<int> InputLinks;
//	data::vector<int> OutputLinks;
//
//	void TriggerAction(Brain* brain);
//
//	virtual void Hello()
//	{
//
//	}
//
//	auto cista_members()
//	{
//		return std::tie(A, InputLinks, OutputLinks);
//	}
//};
//
//struct TestObject2 : TestObject
//{
//	TestObject2() {
//
//	}
//	TestObject2(int a, int b)
//		:TestObject(a)
//	{
//		B = b;
//	}
//
//	float (*NeuronInput)(InputData, Agent*) {};
//	InputData NeuronInputData = InputData{};
//
//	void Hello() override {
//
//	}
//
//	int B = 23;
//	auto cist_members()
//	{
//		return std::tie(A, B, NeuronInput, NeuronInputData);
//	}
//};
//
//struct TestObject3 : TestObject
//{
//	TestObject3()
//	{
//
//	}
//	TestObject3(int a, int c) :
//		TestObject(a)
//	{
//		C = c;
//	}
//
//	int C = 25;
//	auto cist_members()
//	{
//		return std::tie(A, C);
//	}
//};