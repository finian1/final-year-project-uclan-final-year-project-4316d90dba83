#pragma once
#include <variant>
class Agent;
class Cell;

struct EyeData
{
	int ObservationPosX = 0;
	int ObservationPosY = 0;
};

using InputData = std::variant <std::monostate, EyeData>;

//When adding new actions, update the following locations:
//Brain::SaveBrain
//ActionNeuron
//Globals::INPUTTYPE
class Inputs
{
	//static float BaseInput(InputData Data);

public:
	static float NutrientSense(InputData Data, Agent* agent);
	static float AgentSense(InputData Data, Agent* agent);
	static float LavaSense(InputData Data, Agent* agent);
	static float PoisonSense(InputData Data, Agent* agent);
	static float SolidSense(InputData Data, Agent* agent);
	static float HungerSense(InputData Data, Agent* agent);
	static float HeightSense(InputData Data, Agent* agent);
	static float PositionSenseX(InputData Data, Agent* agent);
	static float PositionSenseY(InputData Data, Agent* agent);
};