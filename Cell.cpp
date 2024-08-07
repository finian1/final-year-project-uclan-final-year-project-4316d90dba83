#include "Cell.h"
#include <glad.h>
#include <GLFW/glfw3.h>
#include "Agent.h"
#include "SimGrid.h"
#include "ImGui\imgui.h"

void Cell::Clear()
{
	ClearAgent();
	SetSolid(false);
	mData.mIsLava = false;
	mData.mIsPoisoned = false;
	mData.mPlantNutrients = 0;
	mData.mTerrainHeight = 0.0f;
	mIsDirty = true;
}

void Cell::Render(
	unsigned int ShaderProgramID, 
	bool ShowAgents,
	bool ShowNutrients,
	bool ShowTerrain,
	bool ShowLava,
	bool ShowPoison)
{
	//int vertexColorLocation = glGetUniformLocation(ShaderProgramID, "ourColor");

	if (IsDirty() || 
		ShowAgents != bPrevShowAgents || 
		ShowNutrients != bPrevShowNutrients || 
		ShowTerrain != bPrevShowTerrain ||
		ShowLava != bPrevShowLava ||
		ShowPoison != bPrevShowPoison
		)
	{
		utility::Colour finalColour(0.0f, 0.0f, 0.0f);

		if (mData.mSolid)
		{
			finalColour = utility::Colour(0.6, 0.6, 0.6);
		}
		else if ((mData.mOccupyingAgent == nullptr || !ShowAgents) && ShowTerrain && (mData.mPlantNutrients / mData.mMaxNutrients < 0.3f || !ShowNutrients))
		{
			finalColour = { mData.mTerrainHeight / gMaxTerrainHeight, mData.mTerrainHeight / gMaxTerrainHeight, mData.mTerrainHeight / gMaxTerrainHeight };
		}
		else if ((mData.mOccupyingAgent == nullptr || !ShowAgents) && mData.mIsLava && ShowLava)
		{
			finalColour = { 1.0f, 0.5f, 0.0f };
		}
		else if ((mData.mOccupyingAgent == nullptr || !ShowAgents) && mData.mIsPoisoned && ShowPoison)
		{
			finalColour = { 0.7f, 0.0f, 0.7f };
		}
		else if ((mData.mOccupyingAgent == nullptr || !ShowAgents) && ShowNutrients)
		{
			finalColour.G = mData.mPlantNutrients / mData.mMaxNutrients;
		}
		else if (mData.mOccupyingAgent != nullptr && ShowAgents)
		{
			finalColour = mData.mOccupyingAgent->mAgentColour;
		}
		//else
		//{
		//	finalColour = mColour;
		//}

		int r = finalColour.R * 255.0f;
		int g = finalColour.G * 255.0f;
		int b = finalColour.B * 255.0f;

		unsigned char testr = (r & 0xff);
		unsigned char testg = (g & 0xff);
		unsigned char testb = (b & 0xff);

		unsigned char* data = new unsigned char[3];

		data[0] = (r & 0xff);
		data[1] = (g & 0xff);
		data[2] = (b & 0xff);

		mOwningGrid->gridPixelData[mPosY * mOwningGrid->GetGridSize() * 3 + mPosX * 3] = (r & 0xff);
		mOwningGrid->gridPixelData[mPosY * mOwningGrid->GetGridSize() * 3 + mPosX * 3 + 1] = (g & 0xff);
		mOwningGrid->gridPixelData[mPosY * mOwningGrid->GetGridSize() * 3 + mPosX * 3 + 2] = (b & 0xff);

		glTexSubImage2D(GL_TEXTURE_2D, 0, mPosX, mPosY, 1, 1, GL_RGB, GL_UNSIGNED_BYTE, data);

		delete[] data;
		/*mOwningGrid->gridPixelData[mPosX * 100 * 3 + mPosY * 3] = 0x00;
		mOwningGrid->gridPixelData[mPosX * 100 * 3 + mPosY * 3 + 1] = 0x00;
		mOwningGrid->gridPixelData[mPosX * 100 * 3 + mPosY * 3 + 2] = 0xff;*/

		/*glUniform4f(vertexColorLocation, finalColour.R, finalColour.G, finalColour.B, 1.0f);
		glBindVertexArray(VAO);
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);*/
		mIsDirty = false;
	}
	bPrevShowAgents = ShowAgents;
	bPrevShowNutrients = ShowNutrients;
	bPrevShowTerrain = ShowTerrain;
	bPrevShowLava = ShowLava;
	bPrevShowPoison = ShowPoison;
}

bool Cell::SetAgent(Agent* NewOccupyingAgent)
{
	if (mData.mOccupyingAgent == nullptr)
	{
		mData.mOccupyingAgent = NewOccupyingAgent;
		mIsDirty = true;
		return true;
	}
	return false;
}

bool Cell::ClearAgent()
{
	if (mData.mOccupyingAgent != nullptr)
	{
		mData.mOccupyingAgent = nullptr;
		mIsDirty = true;
		return true;
	}
	return false;
}

bool Cell::HasAgent()
{
	return(mData.mOccupyingAgent != nullptr);
}

void Cell::AddGroundNutrients(float Amount)
{
	mData.mGroundNutrients += Amount;
}

void Cell::AddPlantNutrients(float Amount)
{
	mData.mPlantNutrients += Amount;
	//if (mData.mPlantNutrients > mData.mMaxNutrients)
	//{
	//	mData.mPlantNutrients = mData.mMaxNutrients;
	//}
	mIsDirty = true;
}

void Cell::ConvertGroundToPlantNutrients(float Amount)
{
	if (mData.mPlantNutrients + Amount > mData.mMaxNutrients)
	{
		Amount = (mData.mPlantNutrients + Amount) - mData.mMaxNutrients;
	}
	if (Amount >= mData.mGroundNutrients)
	{
		AddPlantNutrients(mData.mGroundNutrients);
		mData.mGroundNutrients = 0;
	}
	else
	{
		mData.mGroundNutrients -= Amount;
		AddPlantNutrients(Amount);
	}
}

float Cell::RemovePlantNeutrients(float Amount)
{
	float neutrientsRemoved = Amount;
	if (mData.mPlantNutrients - Amount <= 0.0f)
	{
		neutrientsRemoved = mData.mPlantNutrients;
		mData.mPlantNutrients = 0.0f;
	}
	else
	{
		mData.mPlantNutrients -= Amount;
	}
	mIsDirty = true;
	return neutrientsRemoved;
}

bool Cell::GetOccupyingAgent(Agent*& OccupyingAgent)
{
	if (mData.mOccupyingAgent != nullptr)
	{
		OccupyingAgent = mData.mOccupyingAgent;
		return true;
	}
	return false;
}

void Cell::DisplayCellControls()
{
	if (ImGui::Checkbox("Has Lava", &mData.mIsLava) ||
		ImGui::Checkbox("Is Poisoned", &mData.mIsPoisoned) ||
		ImGui::Checkbox("Is Solid", &mData.mSolid))
	{
		mIsDirty = true;
	}
}
