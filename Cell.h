#pragma once
#include <vector>
#include <array>
#include <string>
#include "Globals.h"
#include "Colour.h"
#include "UtilityTypes.h"

class Agent;
class SimGrid;

struct RenderParams
{
	bool mShowNutrients = true;
	bool mShowAgents = true;
	bool mShowLava = true;
	bool mShowPoison = true;
};

struct CellData
{
	float mPlantNutrients = 0.0f;
	float mGroundNutrients = 0.0f;
	float mMaxNutrients = 10.0f;
	float mTerrainHeight = 1.0f;
	bool mSolid = false;
	bool mIsLava = false;
	bool mIsPoisoned = false;
	Agent* mOccupyingAgent = nullptr;
};

class Cell
{
public:
	std::array<float, 16> OriginVertices =
	{ 0.0f, 0.0f, 0.0f   //Top left
	, 0.0f, 0.0f, 0.0f    //Top right
	, 0.0f, 0.0f, 0.0f    //Bottom left
	, 0.0f, 0.0f, 0.0f }; //Bottom right

	Cell(std::array<float, 16> OriginalVertexPositions, int PosX, int PosY, SimGrid* owningGrid)
	{
		OriginVertices = OriginalVertexPositions;
		mPosX = PosX;
		mPosY = PosY;
		mOwningGrid = owningGrid;
	}

	Cell()
	{

	}

	~Cell()
	{
		Clear();
		delete(mData.mOccupyingAgent);
	}

	void Clear();

	void Render(unsigned int ShaderProgramID, 
		bool ShowAgents = true,
		bool ShowNutrients = true,
		bool ShowTerrain = true,
		bool ShowLava = true,
		bool ShowPoison = true);

	void SetColor(float R, float G, float B)
	{
		mColour.R = R;
		mColour.G = G;
		mColour.B = B;

		

		mIsDirty = true;
	}

	CellData GetData()
	{
		return mData;
	}

	void GetCellPos(int& X, int& Y)
	{
		X = mPosX;
		Y = mPosY;
	}

	void SetDirty(bool val)
	{
		mIsDirty = val;
	}

	bool SetAgent(Agent* NewOccupyingAgent);
	bool ClearAgent();
	bool GetOccupyingAgent(Agent*& OccupyingAgent);
	bool HasAgent();
	void AddGroundNutrients(float Amount);
	void AddPlantNutrients(float Amount);
	void ConvertGroundToPlantNutrients(float Amount);
	float RemovePlantNeutrients(float amount);
	void SetTerrainHeight(float val)
	{
		mData.mTerrainHeight = val;
	}

	void SetSolid(bool val)
	{
		mData.mSolid = val;
	}
	void SetLava(bool val)
	{
		mData.mIsLava = val;
	}
	void SetPoison(bool val)
	{
		mData.mIsPoisoned = val;
	}

	bool IsDirty()
	{
		return mIsDirty;
	}

	void DisplayCellControls();

	unsigned int VBO, VAO, EBO;

	utility::Colour mColour = { 0.0f, 0.0f, 0.0f };

private:
	CellData mData;

	SimGrid* mOwningGrid;

	int mPosX;
	int mPosY;

	//Has the cell changed since last render call?
	bool mIsDirty = true;

	bool bPrevShowNutrients = true;
	bool bPrevShowAgents = true;
	bool bPrevShowTerrain = true;
	bool bPrevShowLava = true;
	bool bPrevShowPoison = true;
};

