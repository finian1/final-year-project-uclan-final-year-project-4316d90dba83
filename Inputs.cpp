#include "Inputs.h"
#include "Agent.h"
#include "Cell.h"
#include "SimGrid.h"

//float Inputs::BaseInput(InputData Data)
//{
//	auto& data = std::get<BaseInputData>(Data);
//	return 0.0f;
//}

float Inputs::NutrientSense(InputData Data, Agent* agent)
{
	auto& data = std::get<EyeData>(Data);
	utility::Vector2i pos = agent->GetPosition();
	
	Cell* observedCell = agent->GetGrid()->GetCell(pos.X + data.ObservationPosX, pos.Y + data.ObservationPosY);
	if (observedCell != nullptr)
	{
		CellData cellData = observedCell->GetData();
		float val = cellData.mPlantNutrients / cellData.mMaxNutrients;
		return(val);
	}
	else
	{
		return 0.0f;
	}
}

float Inputs::AgentSense(InputData Data, Agent* agent)
{
	auto& data = std::get<EyeData>(Data);
	utility::Vector2i pos = agent->GetPosition();
	Cell* observedCell = agent->GetGrid()->GetCell(pos.X + data.ObservationPosX, pos.Y + data.ObservationPosY);
	Agent* occupyingAgent = nullptr;
	observedCell->GetOccupyingAgent(occupyingAgent);
	if (observedCell != nullptr && occupyingAgent != nullptr && occupyingAgent != agent)
	{
		return 1.0f;
	}
	else
	{
		return 0.0f;
	}
}

float Inputs::LavaSense(InputData Data, Agent* agent)
{
	auto& data = std::get<EyeData>(Data);
	utility::Vector2i pos = agent->GetPosition();
	Cell* observedCell = agent->GetGrid()->GetCell(pos.X + data.ObservationPosX, pos.Y + data.ObservationPosY);
	if (observedCell != nullptr && observedCell->GetData().mIsLava)
	{
		return 1.0f;
	}
	else
	{
		return 0.0f;
	}
}

float Inputs::PoisonSense(InputData Data, Agent* agent)
{
	auto& data = std::get<EyeData>(Data);
	utility::Vector2i pos = agent->GetPosition();
	Cell* observedCell = agent->GetGrid()->GetCell(pos.X + data.ObservationPosX, pos.Y + data.ObservationPosY);
	if (observedCell != nullptr && observedCell->GetData().mIsPoisoned)
	{
		return 1.0f;
	}
	else
	{
		return 0.0f;
	}
}

float Inputs::SolidSense(InputData Data, Agent* agent)
{
	auto& data = std::get<EyeData>(Data);
	utility::Vector2i pos = agent->GetPosition();
	Cell* observedCell = agent->GetGrid()->GetCell(pos.X + data.ObservationPosX, pos.Y + data.ObservationPosY);
	if (observedCell != nullptr && observedCell->GetData().mSolid)
	{
		return 1.0f;
	}
	else
	{
		return 0.0f;
	}
}

float Inputs::HungerSense(InputData Data, Agent* agent)
{
	if (agent->GetStarterEnergy() <= agent->GetEnergy())
	{
		return 0.0f;
	}
	else
	{
		return 1 - (agent->GetEnergy() / agent->GetStarterEnergy());
	}
}

float Inputs::HeightSense(InputData Data, Agent* agent)
{
	auto& data = std::get<EyeData>(Data);
	utility::Vector2i pos = agent->GetPosition();

	Cell* observedCell = agent->GetGrid()->GetCell(pos.X + data.ObservationPosX, pos.Y + data.ObservationPosY);
	return observedCell->GetData().mTerrainHeight / gMaxTerrainHeight;
}

float Inputs::PositionSenseX(InputData Data, Agent* agent)
{
	//Returns 1 if agent is at max X value, 0 if agent is at minimum X value
	utility::Vector2i pos = agent->GetPosition();
	return 1.0f - float(pos.X) / float(agent->GetGrid()->GetGridSize());
}

float Inputs::PositionSenseY(InputData Data, Agent* agent)
{
	//Returns 1 if agent is at max Y value, 0 if agent is at minimum Y value
	utility::Vector2i pos = agent->GetPosition();
	return 1.0f - float(pos.Y) / float(agent->GetGrid()->GetGridSize());
}