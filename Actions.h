#pragma once
#include <variant>
class Agent;
class Cell;

struct MouthData
{
	float AmountToEat = 0.5f;
};

struct AttackData
{
	float DamageToDeal = 100.0f;
	float Efficiency = 0.5f;
};

struct MovementData
{
	float ClimbingEfficiency = 1.0f;
};

using ActionData = std::variant <std::monostate, MouthData, AttackData, MovementData>;

//When adding new actions, update the following locations:
//Brain::SaveBrain
//ActionNeuron
//Globals::ACTIONTYPE
class Actions
{
	static void BaseAction(Agent* agent, float Cost);

public:
	//Movement actions
	static void MoveForward(ActionData Data, Agent* agent);
	static void TurnRight(ActionData Data, Agent* agent);
	static void TurnLeft(ActionData Data, Agent* agent);

	//Mouth actions
	static void Eat(ActionData Data, Agent* agent);

	//Offensive actions
	static void Attack(ActionData Data, Agent* agent);

};