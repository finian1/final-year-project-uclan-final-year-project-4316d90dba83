#include <algorithm>

#include "Agent.h"
#include "Component.h"
#include "ComponentAction.h"
#include "ComponentInput.h"
#include "SimGrid.h"
#include "Cell.h"
#include "Brain.h"
#include "MouthComponent.h"
#include "MovementComponent.h"
#include "CarnivoreMouthComponent.h"
#include "EyeComponent.h"
#include "Actions.h"

//When copying an agent we need:
//New components with IDs copied from old agent
//Action and input IDs copied from old agent
//Action and input map copied from old agent, with IDs set
//Set local ID counters to current agent ID counters

Agent::Agent(int PosX, int PosY, int ID, int Species, SimGrid* grid, Brain* brain)
{
	mAgentBrain = std::unique_ptr<Brain>{ brain };
	mAgentBrain.get()->SetControllingAgent(this);
	mCurrentGrid = grid;
	mAgentID = ID;
	mSpeciesID = Species;
	mMutationChance = utility::RandomNumber(0.0f, 0.4f);
	SetPosition(utility::Vector2i(PosX, PosY));
	mSurroundingNutrients = mCurrentGrid->GetNutrientsSurroundingCell(mPosition.X, mPosition.Y);
	mSurroundingLava = mCurrentGrid->GetLavaSurroundingCell(mPosition.X, mPosition.Y);
	mSurroundingPoison = mCurrentGrid->GetPoisonSurroundingCell(mPosition.X, mPosition.Y);
}

Agent::Agent(Agent* Parent, int SpawnPosX, int SpawnPosY, int ID, SimGrid* grid)
{
	mAgentColour = Parent->mAgentColour;

	mAgentBrain = std::make_unique<Brain>(this, Parent);

	mComponentIDCounter = Parent->GetComponentIDCounter();
	mCurrentGrid = grid;
	mAgentID = ID;
	mAgentGeneration = Parent->mAgentGeneration + 1;
	mSpeciesID = Parent->mSpeciesID;

	mNutrientSensitivity = Parent->mNutrientSensitivity;
	mHealthSensitivity = Parent->mHealthSensitivity;
	mEnergySensitivity = Parent->mEnergySensitivity;
	mLavaSensitivity = Parent->mLavaSensitivity;
	mPoisonSensitivity = Parent->mPoisonSensitivity;
	mPunishmentSensitivity = Parent->mPunishmentSensitivity;

	if (mAgentGeneration > grid->mHighestGeneration)
	{
		grid->mHighestGeneration = mAgentGeneration;
	}

	float mutationVal = utility::RandomNumber(0.0f, 1.0f);
	if (mutationVal <= mMutationChance)
	{
		Mutate();
	}

	SetPosition(utility::Vector2i(SpawnPosX, SpawnPosY));
	mSurroundingNutrients = mCurrentGrid->GetNutrientsSurroundingCell(mPosition.X, mPosition.Y);
	mSurroundingLava = mCurrentGrid->GetLavaSurroundingCell(mPosition.X, mPosition.Y);
	mSurroundingPoison = mCurrentGrid->GetPoisonSurroundingCell(mPosition.X, mPosition.Y);
}

Agent::Agent(int PosX, int PosY, int ID, int Species, float BrainConnectionDensity, SimGrid* grid)
{
	mAgentBrain = std::make_unique<Brain>(this);

	GenerateRandomComponents();
	AddComponent(new EyeComponent(0, 0));
	AddComponent(new EyeComponent(0, 1));
	int randHiddenLayers = 0/*utility::RandomNumber(0, mAgentBrain->mMaxHiddenLayers)*/;
	if (randHiddenLayers > 0)
	{
		mAgentBrain->AddNewHiddenLayer(randHiddenLayers);
		for (int i = 0; i < randHiddenLayers; i++)
		{
			int randHiddenNeurons = utility::RandomNumber(1, mAgentBrain->mMaxHiddenNeurons);
			for (int j = 0; j < randHiddenNeurons; j++)
			{
				mAgentBrain->AddNewHiddenNeuron(i + 1);
			}
		}
	}

	mCurrentGrid = grid;
	mAgentID = ID;
	mSpeciesID = Species;
	SetPosition(utility::Vector2i(PosX, PosY));
	mAgentBrain->SetConnectionChance(BrainConnectionDensity);
	mAgentBrain->RandomizeLinks();
	//mAgentBrain->SaveBrain("TestBrain");
	//Brain* test = LoadBrain("TestBrain");
}

Agent::~Agent()
{
}

void Agent::Update(float DeltaTime)
{
	//Store previous update values
	mPreviousAgentEnergy = mAgentEnergy;
	mPreviousSurroundingNutrients = mCurrentGrid->GetNutrientsSurroundingCell(mPosition.X, mPosition.Y);
	mPreviousSurroundingLava = mCurrentGrid->GetLavaSurroundingCell(mPosition.X, mPosition.Y);
	mPreviousSurroundingPoison = mCurrentGrid->GetPoisonSurroundingCell(mPosition.X, mPosition.Y);
	mPreviousAgentHealth = mAgentHealth;

	//Apply base health and energy modifiers
	mAgentEnergy -= mBaseEnergyUsage;
	mAgentHealth += mBaseHealthRegen;
	mAgentHealth = fmin(mStartingHealth, mAgentHealth);

	//Triggers any actions
	mAgentBrain->Update();
	//Get data of cells surrounding agent 
	mSurroundingNutrients = mCurrentGrid->GetNutrientsSurroundingCell(mPosition.X, mPosition.Y);
	mSurroundingLava = mCurrentGrid->GetLavaSurroundingCell(mPosition.X, mPosition.Y);
	mSurroundingPoison = mCurrentGrid->GetPoisonSurroundingCell(mPosition.X, mPosition.Y);
	mAgentBrain->UpdateWeights();
	//Updates brain weights
	//Calculating overall energy loss and gain
	if (mPreviousAgentEnergy < mAgentEnergy)
	{
		mEnergyGained += mAgentEnergy - mPreviousAgentEnergy;
	}
	if (mPreviousAgentEnergy > mAgentEnergy)
	{
		mEnergyLost += mPreviousAgentEnergy - mAgentEnergy;
	}

	//Calculating energy efficiency over a period of time
	if (!mSplitLastUpdate)
	{
		mEnergyChanges.push(mAgentEnergy - mPreviousAgentEnergy);
		mEnergyDiffForPeriod += mAgentEnergy - mPreviousAgentEnergy;
		if (mEnergyChanges.size() > mUpdatesForEfficiencyCalc)
		{
			mEnergyDiffForPeriod -= mEnergyChanges.front();
			mEnergyChanges.pop();
		}
		mEfficiency = mEnergyDiffForPeriod / mEnergyChanges.size() + 1;
	}
	//mAgentBrain->AdjustWeights(((mAgentEnergy - mPreviousAgentEnergy) - 0.5f) + ((mCurrentCell->GetData().mPlantNutrients / mCurrentCell->GetData().mMaxNutrients) - 0.5f)/3);

	mTimeAlive += DeltaTime;
	if (mTimeAlive > mCurrentGrid->mLongestTimeAlive)
	{
		mCurrentGrid->mLongestTimeAlive = mTimeAlive;
		if (mCurrentGrid->mAutoSelectOldestAgent)
		{
			mCurrentGrid->mSelectedAgent = this;
		}
	}
	if (mEnergyGained > mCurrentGrid->mHighestEnergyGained)
	{
		mCurrentGrid->mHighestEnergyGained = mEnergyGained;
		if (mCurrentGrid->mAutoSelectHighestEnergyGain)
		{
			mCurrentGrid->mSelectedAgent = this;
		}
	}
	if (mEfficiency > mCurrentGrid->mHighestEfficiency && mEnergyGained > 200.0f)
	{
		mCurrentGrid->mHighestEfficiency = mEfficiency;
		if (mCurrentGrid->mAutoSelectHighestEfficiency)
		{
			mCurrentGrid->mSelectedAgent = this;
		}
	}

	mSplitLastUpdate = false;
	if (mAgentEnergy >= mEnergyRequiredToSplit)
	{
		Agent* newAgent = nullptr;
		switch (utility::RandomNumber(0, 3))
		{
		case(0):
			newAgent = mCurrentGrid->AddAgent(this, mPosition.X + 1, mPosition.Y);
			break;
		case(1):
			newAgent = mCurrentGrid->AddAgent(this, mPosition.X - 1, mPosition.Y);
			break;
		case(2):
			newAgent = mCurrentGrid->AddAgent(this, mPosition.X, mPosition.Y + 1);
			break;
		case(3):
			newAgent = mCurrentGrid->AddAgent(this, mPosition.X, mPosition.Y - 1);
			break;
		}
		
		if (newAgent != nullptr)
		{
			mAgentEnergy -= mStartingEnergy;
			newAgent->mAgentEnergy = mStartingEnergy;
			mSplitLastUpdate = true;
		}
	}
	if (mWaste >= mAmountOfWasteBeforePoop)
	{
		ExpelWaste();
	}
	if (mAgentEnergy <= 0.0f || mAgentHealth <= 0.0f)
	{
		Die();
	}
}

void Agent::ExpelWaste()
{
	mCurrentCell->AddGroundNutrients(mWaste * (1 - mPercentOfWasteIsUsable));
	mCurrentCell->AddPlantNutrients(mWaste * mPercentOfWasteIsUsable);
	mWaste = 0;
}

void Agent::Mutate()
{
	int mutationType = utility::RandomNumber(0, 2);
	switch (mutationType)
	{
	case(0):
	{
		mAgentBrain.get()->CreateRandomLink();
		break;
	}
	case(1):
	{
		mAgentBrain.get()->DestroyRandomLink();
		break;
	}
	case(2):
	{
		mAgentBrain.get()->SplitRandomLink();
		break;
	}
	}
}

void Agent::GenerateRandomComponents()
{
	int numOfComponents = utility::RandomNumber(1, mAgentBrain->mMaxNumOfInitComponents);
	for (int i = 0; i < numOfComponents; i++)
	{
		int randComponentToAdd = utility::RandomNumber(1, 7);
		switch (randComponentToAdd)
		{
		case(1):
		{
			AddComponent(new MovementComponent(utility::RandomNumber(0.1f, 1.0f)));
			break;
		}
		case(2):
		{
			AddComponent(new MouthComponent(0.5f));
			break;
		}
		case(3):
		{
			int eyePosX = utility::RandomNumber(-mAgentBrain->mMaxEyeViewDist, mAgentBrain->mMaxEyeViewDist);
			int eyePosY = utility::RandomNumber(-mAgentBrain->mMaxEyeViewDist, mAgentBrain->mMaxEyeViewDist);
			AddComponent(new EyeComponent(eyePosX, eyePosY));
			break;
		}
		case(4):
		{
			AddComponent(new CarnivoreMouthComponent(100.0f));
			break;
		}
		case(5):
		{
			mAgentBrain->AddNewSensorNeuron(gInputs.HungerSense, {}, HUNGERSENSE);
			break;
		}
		case(6):
		{
			mAgentBrain->AddNewSensorNeuron(gInputs.PositionSenseX, {}, POSITIONSENSEX);
			break;
		}
		case(7):
		{
			mAgentBrain->AddNewSensorNeuron(gInputs.PositionSenseY, {}, POSITIONSENSEY);
			break;
		}
		}
	}
}

void Agent::AddComponent(Component* NewComponent)
{
	mAgentBrain->AddNewComponentNeurons(NewComponent);

	mAttachedComponents.insert(std::pair<int, Component*>(mComponentIDCounter, NewComponent));
	mComponentIDCounter++;
}

void Agent::RemoveComponent(int ComponentID)
{

}

utility::Vector2i Agent::GetPosition()
{
	return mPosition;
}

void Agent::SetPosition(utility::Vector2i pos)
{
	if (mCurrentGrid == nullptr)
	{
		mPosition = pos;
		return;
	}

	Cell* mTargetCell = mCurrentGrid->GetCell(pos.X, pos.Y);
	if (mTargetCell != nullptr && !mTargetCell->HasAgent() && !mTargetCell->GetData().mSolid)
	{
		if (mTargetCell->SetAgent(this))
		{
			if (mCurrentCell != nullptr)
			{
				mCurrentCell->ClearAgent();
			}
			mCurrentCell = mTargetCell;
		}
		mCurrentCell->GetCellPos(mPosition.X, mPosition.Y);
	}
}

void Agent::Move(int X, int Y)
{
	SetPosition(utility::Vector2i( mPosition.X + X, mPosition.Y + Y ));
	if (mCurrentCell->GetData().mIsLava)
	{
		mTimesRanIntoLava ++;
		RemoveHealth(mLavaDamage);
	}
}

void Agent::Die()
{
	ExpelWaste();
	GetGrid()->MarkAgentForRemoval(this);
}

float Agent::GetActionReward()
{
	float val = 0.0f;
	if (!mSplitLastUpdate)
	{
		val += (mAgentEnergy - mPreviousAgentEnergy) * mEnergySensitivity;
	}
	val += (mAgentHealth - mPreviousAgentHealth) * mHealthSensitivity;
	val += (mSurroundingNutrients - mPreviousSurroundingNutrients) * mNutrientSensitivity;
	val += (mPreviousSurroundingLava - mSurroundingLava) * mLavaSensitivity;
	val += (mPreviousSurroundingPoison - mSurroundingPoison) * mPoisonSensitivity;
	if (val < 0)
	{
		val *= mPunishmentSensitivity;
	}
	return val;
}

bool Agent::ShouldLearn()
{
	return mCurrentGrid->mShouldAgentsLearn;
}

Layer Agent::GetBrainLayer(int id)
{
	return *mAgentBrain->GetLayer(id);
}

data::vector<int> Agent::GetBrainLayerOrder()
{
	return mAgentBrain->GetLayerOrder();
}

