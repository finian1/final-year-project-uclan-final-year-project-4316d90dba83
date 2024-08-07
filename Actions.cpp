#include "Actions.h"
#include "Agent.h"
#include "Cell.h"
#include "SimGrid.h"
#include "Brain.h"

void Actions::BaseAction(Agent* agent, float Cost)
{
	agent->RemoveEnergy(Cost);
}

void Actions::MoveForward(ActionData Data, Agent* agent)
{
	auto& data = std::get<MovementData>(Data);
	float cost = 0.05f;

	utility::Vector2i ownerDir = agent->GetDirection();
	utility::Vector2i ownerPos = agent->GetPosition();
	Cell* directionCell = agent->GetGrid()->GetCell(ownerPos.X + ownerDir.X, ownerPos.Y + ownerDir.Y);
	Cell* currentCell = agent->GetCell();
	float heightDiff = directionCell->GetData().mTerrainHeight - currentCell->GetData().mTerrainHeight;
	if (heightDiff > 0.0f)
	{
		cost += heightDiff * (1.0f / data.ClimbingEfficiency);
	}
	BaseAction(agent, cost);
	agent->Move(ownerDir.X, ownerDir.Y);
}

void Actions::TurnRight(ActionData Data, Agent* agent)
{
	BaseAction(agent, 0.05f);
	agent->TurnRight();
}

void Actions::TurnLeft(ActionData Data, Agent* agent)
{
	BaseAction(agent, 0.05f);
	agent->TurnLeft();
}

void Actions::Eat(ActionData Data, Agent* agent)
{
	auto& data = std::get<MouthData>(Data);
	BaseAction(agent, 0.05f);
	if (!agent->mAgentBrain.get()->mSimulating)
	{
		if (agent->GetCell()->GetData().mIsPoisoned)
		{
			agent->EatenPoison();
			agent->RemoveHealth(agent->GetCell()->RemovePlantNeutrients(data.AmountToEat));
		}
		else
		{
			agent->AddEnergy(agent->GetCell()->RemovePlantNeutrients(data.AmountToEat));
		}
	}
	else
	{
		if (agent->GetCell()->GetData().mIsPoisoned)
		{
			agent->RemoveHealth(data.AmountToEat);
		}
		else
		{
			agent->AddEnergy(data.AmountToEat);
		}
	}
}

void Actions::Attack(ActionData Data, Agent* agent)
{
	auto& data = std::get<AttackData>(Data);
	BaseAction(agent, 0.05f);
	utility::Vector2i ownerDir = agent->GetDirection();
	utility::Vector2i ownerPos = agent->GetPosition();
	Cell* attackingCell = agent->GetGrid()->GetCell(ownerPos.X + ownerDir.X, ownerPos.Y + ownerDir.Y);
	Agent* targetAgent;
	if (attackingCell->GetOccupyingAgent(targetAgent))
	{
		if (!agent->mAgentBrain.get()->mSimulating)
		{
			agent->AddEnergy(targetAgent->RemoveEnergy(data.DamageToDeal) * data.Efficiency);
		}
		else
		{
			agent->AddEnergy(data.DamageToDeal * data.Efficiency);
		}
	}
}