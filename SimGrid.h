#pragma once
#include <vector>
#include <array>
#include <map>
#include <unordered_map>
#include <queue>
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>

#include "Globals.h"

class Cell;
class Agent;
class Brain;

struct AgentInitData
{
	//Sensitivity
	float MinNutrientSensitivity = 0.0f;
	float MaxNutrientSensitivity = 1.0f;
	float MinHealthSensitivity = 0.0f;
	float MaxHealthSensitivity = 1.0f;
	float MinEnergySensitivity = 0.0f;
	float MaxEnergySensitivity = 1.0f;
	float MinLavaSensitivity = 0.0f;
	float MaxLavaSensitivity = 1.0f;
	float MinPoisonSensitivity = 0.0f;
	float MaxPoisonSensitivity = 1.0f;
	float MinPunishmentSensitivity = 5.0f;
	float MaxPunishmentSensitivity = 20.0f;
	//mutation
	float MinMutationChance = 0.0f;
	float MaxMutationChance = 1.0f;
	//Network
	float MinNetworkConnectivity = 0.5f;
	float MaxNetworkConnectivity = 1.0f;
	int MinUpdatesForRandomAction = 5;
	int MaxUpdatesForRandomAction = 15;
};

class SimGrid
{
public:

	SimGrid();

	~SimGrid();

	void InitGrid(
		int CellCountX, int CellCountY,  
		float DefaultStartPositionX, float DefaultStartPositionY);

	void ResetGrid();

	void Update(float DeltaTime);

	void RenderGrid();
	void RenderGraphs();
	void RenderNeuralNetwork();
	void PrintAgentData();
	void PrintCellData();
	void PrintGridControls();

	void AddCell(float StartPosX, float StartPosY, int PosX, int PosY);

	void MarkAgentForRemoval(Agent* AgentToRemove)
	{
		mAgentsToRemove.push_back(AgentToRemove);
	}
	AgentInitData mDefaultAgentInitData;
	Agent* AddAgent(int PosX, int PosY);
	Agent* AddAgent(Agent* Parent, int PosX, int PosY);
	Agent* AddAgentAtRandomPos();
	void AddNutrients(int PosX, int PosY, float Amount);
	void AddGroundNutrients(int PosX, int PosY, float Amount);
	void GrowPlantNutrients(int PosX, int PosY, float Amount);
	float GetNutrientsSurroundingCell(int PosX, int PosY);
	int GetLavaSurroundingCell(int PosX, int PosY);
	int GetPoisonSurroundingCell(int PosX, int PosY);
	Agent* GetAgentAtCell(int PosX, int PosY);
	void RemoveAgent(Agent* mAgentToRemove);
	Cell* GetCell(int X, int Y);
	void LoadShader();

	void GenerateTerrain();

	void ZoomIn(float amount);
	void ScrollRight(float amount);
	void ScrollDown(float amount);
	int GetGridSize()
	{
		return mSizeX;
	}

	Agent* CreateNewAgentWithBrain(std::string FileName);
	Brain* LoadBrain(std::string FileName);

	void GetOffset(float& offsetX, float& offsetY)
	{
		offsetX = gridOffsetX;
		offsetY = gridOffsetY;
	}
	float GetZoom()
	{
		return mZoom;
	}

	bool IsPositionAtEdge(int PosX, int PosY)
	{
		if (PosX == 0 || PosY == 0)
		{
			return true;
		}
		if (PosX == mSizeX || PosY == mSizeY)
		{
			return true;
		}
		return false;
	}

	int GetNumberOfSpecies()
	{
		return mSpeciesCount.size();
	}

	void AgentWalkedIntoLava()
	{
		mTimesRanIntoLava++;
	}
	void AgentAtePoison()
	{
		mTimesEatenPoison++;
	}

	//User option variables
	bool mIsPaused = false;
	bool mShowNutrients = true;
	bool mShowLava = true;
	bool mShowPoison = true;
	bool mShowAgents = true;
	bool mShowTerrain = false;
	bool mAutoSelectOldestAgent = false;
	bool mAutoSelectHighestEnergyGain = false;
	bool mAutoSelectHighestEfficiency = false;
	bool mShowGraphs = false;
	bool mShowSpeciesGraph = false;
	bool mShowEfficiency1SumGraph = false;
	bool mShowEfficiencyGraph = false;
	bool mShowUniqueSpeciesNumGraph = false;
	bool mShowLavaGraph = false;
	bool mShowPoisonGraph = false;
	bool mShouldAgentsLearn = true;
	bool mRandomizeAgentValues = false;
	//

	bool Initialized = false;
	bool ShaderLoaded = false;
	//Grid statistics
	int mAliveAgents = 0;
	float mLongestTimeAlive = 0.0f;
	float mHighestEnergyGained = 0.0f;
	float mHighestEfficiency = 0.0f;

	float mAverageLearningRate = 0.0f;
	float mAverageNutrientSensitivity = 0.0f;
	float mAverageEnergySensitivity = 0.0f;
	float mAverageHealthSensitivity = 0.0f;
	float mAverageLavaSensitivity = 0.0f;
	float mAveragePoisonSensitivity = 0.0f;
	float mAveragePunishmentSensitivity = 0.0f;
	float mAverageMutationRate = 0.0f;
	float mAverageRandomActionUpdate = 0.0f;

	float mAverageEfficiencyOfTopAgents = 0.0f;
	int mTimesRanIntoLava = 0;
	int mTimesEatenPoison = 0;
	int mHighestGeneration = 0;

	Cell* mSelectedCell = nullptr;
	Agent* mSelectedAgent = nullptr;

	bool mShouldAutomaticallyStop = false;
	int mUpdatesToSimulate = 100000;

protected:
	//Grid Settings
	int mUpdatesPerSecond = 100;

	int mSizeX = 500;
	int mSizeY = 500;

	float mCellSize = 0.0f;

	//Ticks between random nutrient spawns
	int mTicksBetweenNutrientSpawns = 30;
	int mNumOfNutrientsToSpawn = 2000;
	float mMaxSpawnedNutrientValue = 5.0f;
	float mMinSpawnedNutrientValue = 0.5f;
	int mNutrientSpawnTickCounter = 0;

	//Ticks between random agent spawns
	int mTicksBetweenAgentSpawns = 40;
	int mNumOfAgentstoSpawn = 5;
	int mAgentSpawnTickCounter = 0;
	int mMaxNumberOfAgentsForSpawn = 100;
	int mMinimumSpeciesDiversity = 50;

	//Grid Variables
	int mAgentIDCount = 0;
	int mSpeciesIDCount = 0;
	float mUpdateTimer = 0.0f;
	bool mWrapAround = true;

	std::vector<Cell*> Cells = {};
	std::map<int, Agent*> Agents = {};
	std::unordered_map<int, int> mSpeciesCount = {};
	std::map<int, int> mCompoundedSpeciesCount = {};
	std::vector<Agent*> mAgentsToRemove = {};

	float mSolidDensity = 0.05f;
	float mPoisonDensity = -1.0f;
	float mLavaDensity = -1.0f;
	float mInitialNutrientDensity = 1.0f;
	float mMaxDefaultNutrientSpawn = 5.0f;
	float mMinDefaultNutrientSpawn = 0.0f;
	float mMaxDefaultGroundNutrientSpawn = 10.0f;
	float mMinDefaultGroundNutrientSpawn = 0.0f;

	//Rendering variables
	unsigned int VBO, VAO, EBO;

	std::string vertexCode;
	std::string pixelCode;

	unsigned int mShaderProgramID;
	unsigned int mOffsetLoc;
	unsigned int mZoomLoc;

	float gridOffsetX = 0.0f;
	float gridOffsetY = 0.0f;
	float mZoom = 1.0f;
	float mZoomSensitivity = 0.2f;
	float mScrollSensitivity = 0.01f;

	//Graph variables
	std::vector<float> mAvgEnergySnapshots;
	std::vector<int> mNumOfAgentsAbove1EfficiencySnapshots;
	std::vector<int> mNumOfUniqueAgentSpeciesSnapshots;
	std::vector<float> mNumOfTimesEatenPoisonSnapshots;
	std::vector<float> mNumOfTimesWalkedIntoLavaSnapshots;

	std::vector<float> mAverageNutrientSensitivitySnapshots;
	std::vector<float> mAverageEnergySensitivitySnapshots;
	std::vector<float> mAverageHealthSensitivitySnapshots;
	std::vector<float> mAverageLavaSensitivitySnapshots;
	std::vector<float> mAveragePoisonSensitivitySnapshots;
	std::vector<float> mAveragePunishmentSensitivitySnapshots;
	std::vector<float> mAverageMutationChanceSnapshots;
	std::vector<float> mAverageUpdatesBetweenRandomActionSnapshots;

	const int mUpdatesBetweenHazardChecks = 100;
	int mUpdatesSinceLastHazardCheck = 0;
	float mCurrentMaxNumOfAgentsAbove1Efficiency = 0;
	int mCurrentMaxUniqueSpeciesCount = 0;
	int mUpdateCount = 0;

	float mMinimumAgeForAgentEfficiency = 10.0f;

	//Network Renderer Variables



public:
	unsigned char* gridPixelData;
private:

	const float vertices[32] =
	{
		//positions               colors             tex coords
		1.0f,  1.0f,  0.0f,   1.0f,  0.0f,  0.0f,   1.0f,  1.0f, //top right
		1.0f, -1.0f,  0.0f,   0.0f,  1.0f,  0.0f,   1.0f,  0.0f, //bottom right
	   -1.0f, -1.0f,  0.0f,   0.0f,  0.0f,  1.0f,   0.0f,  0.0f, //bottom left
	   -1.0f,  1.0f,  0.0f,   1.0f,  1.0f,  0.0f,   0.0f,  1.0f, //top left
	};

	const unsigned int indices[6] = {
		0, 1, 3, // first triangle
		1, 2, 3  // second triangle
	};

	
};

