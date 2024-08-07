#pragma once
#include <vector>
#include <unordered_map>
#include <memory>
#include <queue>
#include <cista.h>
#include "Colour.h"
#include "UtilityTypes.h"
#include "UtilityFunctions.h"

namespace data = cista::offset;

class Component;
class ComponentAction;
class ComponentInput;
class SimGrid;
class Cell;
class Brain;
struct Neuron;
struct Layer;

class Agent
{
public:
	Agent(int PosX, int PosY, int ID, int Species, SimGrid* grid, Brain* brain);
	Agent(int PosX, int PosY, int ID, int Species, float BrainConnectivity, SimGrid* grid);
	Agent(Agent* Parent, int SpawnPosX, int SpawnPosY, int ID, SimGrid* grid);
	~Agent();

	void Update(float DeltaTime);

	//Public functions
	void GenerateRandomComponents();
	void AddComponent(Component* NewComponent);
	void RemoveComponent(int ComponentID);
	void Mutate();
	void Move(int X, int Y);
	void ExpelWaste();
	void Die();
	void RandomizeSensitivity();
	void PrintAgentControls();

	float GetActionReward();

	void AddEnergy(float val){mAgentEnergy += val;}
	float RemoveEnergy(float val)
	{
		float mEnergy = mAgentEnergy;
		mAgentEnergy = fmax(mAgentEnergy - val, 0);
		mWaste += val;
		return mEnergy - mAgentEnergy;
	}
	void RemoveHealth(float val){mAgentHealth -= val;}
	void TurnRight()
	{
		mCurrentDirection = (mCurrentDirection + 1) % 4;
	}
	void TurnLeft()
	{
		mCurrentDirection--;
		if (mCurrentDirection < 0)
		{
			mCurrentDirection = 3;
		}
	}


	//Getters
	utility::Vector2i GetPosition();
	int GetComponentIDCounter(){return mComponentIDCounter;}
	SimGrid* GetGrid(){return mCurrentGrid;}
	Cell* GetCell(){return mCurrentCell;}
	int GetID(){return mAgentID;}
	float GetStarterEnergy(){return mStartingEnergy;}
	float GetEnergy(){return mAgentEnergy;}
	void SetEnergy(float amount){mAgentEnergy = amount;}
	float GetHealth(){return mAgentHealth;}
	void SetHealth(float amount){mAgentHealth = amount;}
	float GetEfficiency(){return mEfficiency;}
	float GetPreviousEnergy(){return mPreviousAgentEnergy;}
	utility::Vector2i GetDirection(){return Directions[mCurrentDirection];}
	int GetSpeciesID(){return mSpeciesID;}
	float GetTimeAlive() { return mTimeAlive; }
	//Setters
	void SetPosition(utility::Vector2i pos);
	void SetGrid(SimGrid& CurrentGrid){mCurrentGrid = &CurrentGrid;}
	void SetCell(Cell* CurrentCell){mCurrentCell = CurrentCell;}
	bool ShouldLearn();

	Layer GetBrainLayer(int id);
	data::vector<int> GetBrainLayerOrder();

	//Public variables
	utility::Colour mAgentColour = { 1.0f, 1.0f, 0.0f };
	const utility::Vector2i Directions[4] =
	{
		{0,  1}, //up
		{1,  0}, //right
		{0, -1}, //down
		{-1, 0}  //left
	};
	int mCurrentDirection = 0;
	std::unique_ptr<Brain> mAgentBrain;
	int mAgentGeneration = 0;


	void EatenPoison(){mTimesEatenPoison++;}
	int mTimesRanIntoLava = 0;
	int mTimesEatenPoison = 0;

	void SetNutrientSensitivity(float val)		{ mNutrientSensitivity = val; }
	float GetNutrientSensitivity()				{ return mNutrientSensitivity; }
	void SetEnergySensitivity(float val)		{ mEnergySensitivity = val; }
	float GetEnergySensitivity()				{ return mEnergySensitivity; }
	void SetHealthSensitivity(float val)		{ mHealthSensitivity = val; }
	float GetHealthSensitivity()				{ return mHealthSensitivity; }
	void SetLavaSensitivity(float val)			{ mLavaSensitivity = val; }
	float GetLavaSensitivity()					{ return mLavaSensitivity; }
	void SetPoisonSensitivity(float val)		{ mPoisonSensitivity = val; }
	float GetPoisonSensitivity()				{ return mPoisonSensitivity; }
	void SetPunishmentSensitivity(float val)	{ mPunishmentSensitivity = val; }
	float GetPunishmentSensitivity()			{ return mPunishmentSensitivity; }
	void SetMutationChance(float val)			{ mMutationChance = val; }
	float GetMutationChance()					{ return mMutationChance; };

private:

	const float mLavaDamage = 1.0f;

	float mBaseEnergyUsage = 0.05f;
	float mBaseHealthRegen = 0.5f;

	//int mPositionX;
	//int mPositionY;
	utility::Vector2i mPosition;

	unsigned int mAgentID = 0;
	unsigned int mSpeciesID = 0;

	float mTimeAlive;

	float mStartingEnergy = 100.0f;
	float mStartingHealth = 100.0f;
	float mAgentEnergy = mStartingEnergy;
	float mPreviousAgentEnergy = mAgentEnergy;
	float mAgentHealth = mStartingHealth;
	float mPreviousAgentHealth = mAgentHealth;
	float mWaste = 0.0f;
	float mEnergyGained = 0.0f;
	float mEnergyLost = 0.0f;
	float mEfficiency = 0.0f;

	float mSurroundingNutrients = 0.0f;
	float mPreviousSurroundingNutrients = 0.0f;
	int mPreviousSurroundingLava = 0;
	int mSurroundingLava = 0;
	int mPreviousSurroundingPoison = 0;
	int mSurroundingPoison = 0;
	
	int mUpdatesForEfficiencyCalc = 50;
	float mEnergyDiffForPeriod = 0.0f;
	std::queue<float> mEnergyChanges;
	//void UpdatePossibleActions();
	//void UpdateBrainInputs();

	//When handling the brain:
	//We need to get all inputs from each component
	//Inputs should be the same order each time. Currently easy to do as components can't be removed

	//We're going to want all inputs and actions to be handled directly by the brain.
	std::unordered_map<int, Component*> mAttachedComponents;

	int mComponentIDCounter = 0;

	SimGrid* mCurrentGrid = nullptr;
	Cell* mCurrentCell = nullptr;

	float mEnergyRequiredToSplit = 200.0f;
	float mMutationChance = 0.05f;

	float mPercentOfWasteIsUsable = 0.05f;
	float mAmountOfWasteBeforePoop = 30.0f;

	//Reward values
	float mNutrientSensitivity = 0.1f;
	float mHealthSensitivity = 0.1f;
	float mEnergySensitivity = 1.0f;
	float mLavaSensitivity = 0.1f;
	float mPoisonSensitivity = 0.1f;
	float mPunishmentSensitivity = 10.0f;


	//Mutations:
	//Add link
	//Add sensor
	//Add output
	//Remove link
	//Split link with hidden neuron
	bool mSplitLastUpdate = false;
};

