#include "SimGrid.h"
#include <glad.h>
#include <GLFW/glfw3.h>
#include <filesystem>
#include "Cell.h"
#include "Brain.h"
#include "Agent.h"
#include "MovementComponent.h"
#include "UtilityFunctions.h"
#include "ImGui/imgui.h"
#include "ImGui/imgui_impl_opengl3.h"
#include "ImGui/imgui_impl_glfw.h"
#include "implot.h"
#include "stb_image.h"


using namespace std;

SimGrid::SimGrid()
{
}

SimGrid::~SimGrid()
{
	for (int i = 0; i < Cells.size(); i++)
	{
		delete(Cells[i]);
	}
	Cells.clear();
}

//Prepare shader for rendering
void SimGrid::LoadShader()
{
	std::ifstream vShaderFile;
	std::ifstream pShaderFile;
	// ensure ifstream objects can throw exceptions:
	vShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
	pShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
	try
	{
		vShaderFile.open("./TextureVertexShader.hlsl");
		pShaderFile.open("./TexturePixelShader.hlsl");
		std::stringstream vShaderStream, pShaderStream;

		vShaderStream << vShaderFile.rdbuf();
		pShaderStream << pShaderFile.rdbuf();

		vShaderFile.close();
		pShaderFile.close();

		vertexCode = vShaderStream.str();
		pixelCode = pShaderStream.str();
	}
	catch (std::ifstream::failure e)
	{
		std::cout << "ERROR::SHADER::FILE_NOT_SUCCESFULLY_READ" << std::endl;
	}

	const char* vShaderCode = vertexCode.c_str();
	const char* pShaderCode = pixelCode.c_str();
	// 2. compile shaders
	unsigned int vertex, pixel;
	int success;
	char infoLog[512];

	// vertex Shader
	vertex = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertex, 1, &vShaderCode, NULL);
	glCompileShader(vertex);

	// print compile errors if any
	glGetShaderiv(vertex, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		glGetShaderInfoLog(vertex, 512, NULL, infoLog);
		std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
	};

	pixel = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(pixel, 1, &pShaderCode, NULL);
	glCompileShader(pixel);

	glGetShaderiv(pixel, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		glGetShaderInfoLog(pixel, 512, NULL, infoLog);
		std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
	};

	// shader Program
	mShaderProgramID = glCreateProgram();
	glAttachShader(mShaderProgramID, vertex);
	glAttachShader(mShaderProgramID, pixel);
	glLinkProgram(mShaderProgramID);
	// print linking errors if any
	glGetProgramiv(mShaderProgramID, GL_LINK_STATUS, &success);
	if (!success)
	{
		glGetProgramInfoLog(mShaderProgramID, 512, NULL, infoLog);
		std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
	}

	// delete the shaders as they're linked into our program now and no longer necessary
	glDeleteShader(vertex);
	glDeleteShader(pixel);


	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glGenBuffers(1, &EBO);
	// bind the Vertex Array Object first, then bind and set vertex buffer(s), and then configure vertex attributes(s).
	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	// position attribute
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	// color attribute
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);
	// texture coord attribute
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
	glEnableVertexAttribArray(2);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	//Setting up texture data
	glGenTextures(1, &gGridTexture);
	glBindTexture(GL_TEXTURE_2D, gGridTexture);

	// set the texture wrapping parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);	// set texture wrapping to GL_REPEAT (default wrapping method)
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	// set texture filtering parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, GetGridSize(), GetGridSize(), 0, GL_RGB, GL_UNSIGNED_BYTE, gridPixelData);
	//glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
	glGenerateMipmap(GL_TEXTURE_2D);
	mOffsetLoc = glGetUniformLocation(mShaderProgramID, "offset");
	mZoomLoc = glGetUniformLocation(mShaderProgramID, "zoom");

	ShaderLoaded = true;
}

void SimGrid::GenerateTerrain()
{
	const int smoothingMaskSize = 8;
	utility::Vector2i smoothingMask[smoothingMaskSize] = {
		{-1,1}, {0,1}, {1,1},
		{-1,0},        {1,0},
		{-1,-1},{0,-1},{1,-1}
	};

	std::vector<float> row(GetGridSize(), 0.0f);
	std::vector<std::vector<float>> averageVals(GetGridSize(), row);

	for (int y = 0; y < mSizeX; y++)
	{
		for (int x = 0; x < mSizeY; x++)
		{
			GetCell(x, y)->SetTerrainHeight(utility::RandomNumber(0.0f, gMaxTerrainHeight));
		}
	}

	for (int smoothingNum = 0; smoothingNum < gTerrainSmoothing; smoothingNum++)
	{
		for (int y = 0; y < mSizeX; y++)
		{
			for (int x = 0; x < mSizeY; x++)
			{
				float sum = 0.0f;
				for (int j = 0; j < smoothingMaskSize; j++)
				{
					sum += GetCell(x + smoothingMask[j].X, y + smoothingMask[j].Y)->GetData().mTerrainHeight;
				}
				float average = sum / smoothingMaskSize;
				averageVals[x][y] = average;
			}
		}
		for (int y = 0; y < mSizeX; y++)
		{
			for (int x = 0; x < mSizeY; x++)
			{
				GetCell(x, y)->SetTerrainHeight(averageVals[x][y]);
			}
		}
	}

	/*for (int y = 0; y < mSizeX; y++)
	{
		for (int x = 0; x < mSizeY; x++)
		{
			float ridgeOffset = 1.0f;
			int octaves = 10;
			float lacunarity = 2.0f;
			float gain = 0.5f;
			float sum = 0.0f;
			float freq = 1.0f;
			float ampli = 1.0f;
			float prev = 1.0f;
			for (int i = 0; i < octaves; i++)
			{
				float val = averageVals[x][y];
				float h = ridgeOffset - abs(val);
				float n = h * h;
				sum += n * ampli * prev;
				prev = n;
				freq *= lacunarity;
				ampli *= gain;
			}
			GetCell(x, y)->SetTerrainHeight(sum);
		}
	}*/

}

void SimGrid::ZoomIn(float amount)
{
	mZoom += amount * mZoomSensitivity;
	if (mZoom <= 0.0f)
	{
		mZoom = 0.01f;
	}
}

void SimGrid::ScrollRight(float amount)
{
	gridOffsetX += amount * mScrollSensitivity;
}

void SimGrid::ScrollDown(float amount)
{
	gridOffsetY += amount * mScrollSensitivity;
}

void SimGrid::InitGrid(
	int CellCountX, int CellCountY,
	float DefaultStartPositionX, float DefaultStartPositionY)
{
	
	gRandomNumberGen.seed(gInitSeed);
	mCellSize = 2.0f / GetGridSize();

	//mSizeX = CellCountX;
	//mSizeY = CellCountY;

	int VertexCount = (mSizeX + 1 * mSizeY + 1);
	int CurrentCell = 0;

	gridPixelData = new unsigned char[GetGridSize() * GetGridSize() * 3];
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, GetGridSize(), GetGridSize(), 0, GL_RGB, GL_UNSIGNED_BYTE, gridPixelData);
	glGenerateMipmap(GL_TEXTURE_2D);

	for (int y = 0; y < mSizeY; y++)
	{
		for (int x = 0; x < mSizeX; x++)
		{
			if (GetCell(x, y) == nullptr)
			{
				AddCell(DefaultStartPositionX, DefaultStartPositionY, x, y);
			}
			if (x == 0 || y == 0 || x == GetGridSize() - 1 || y == GetGridSize() - 1)
			{
				Cells[CurrentCell]->SetSolid(true);
			}
			if (utility::RandomNumber(0.0f, 1.0f) < mSolidDensity)
			{
				Cells[CurrentCell]->SetSolid(true);
			}
			if (utility::RandomNumber(0.0f, 1.0f) < mLavaDensity)
			{
				Cells[CurrentCell]->SetLava(true);
			}
			if (utility::RandomNumber(0.0f, 1.0f) < mPoisonDensity)
			{
				Cells[CurrentCell]->SetPoison(true);
			}
			Cells[CurrentCell]->SetColor(0.0f, 0.0f, 0.0f);
			if (utility::RandomNumber(0.0f, 1.0f) < mInitialNutrientDensity)
			{
				Cells[CurrentCell]->AddPlantNutrients(utility::RandomNumber(mMinDefaultNutrientSpawn, mMaxDefaultNutrientSpawn));
			}
			Cells[CurrentCell]->AddGroundNutrients(utility::RandomNumber(mMinDefaultGroundNutrientSpawn, mMaxDefaultGroundNutrientSpawn));
			Cells[CurrentCell]->SetDirty(true);
			CurrentCell++;
		}
	}
	GenerateTerrain();
	Initialized = true;
}

void SimGrid::ResetGrid()
{
	for (std::pair<int, Agent*> agentPair : Agents)
	{
		mAgentsToRemove.push_back(agentPair.second);
	}
	for (Agent* agent : mAgentsToRemove)
	{
		RemoveAgent(agent);
	}
	for (int i = 0; i < Cells.size(); i++)
	{
		delete(Cells[i]);
	}
	Cells.clear();
	mAgentsToRemove.clear();
	mHighestGeneration = 0;
	mAvgEnergySnapshots.clear();
	mNumOfAgentsAbove1EfficiencySnapshots.clear();
	mNumOfUniqueAgentSpeciesSnapshots.clear();
	mNumOfTimesEatenPoisonSnapshots.clear();
	mNumOfTimesWalkedIntoLavaSnapshots.clear();

	mAverageNutrientSensitivitySnapshots.clear();
	mAverageEnergySensitivitySnapshots.clear();
	mAverageHealthSensitivitySnapshots.clear();
	mAverageLavaSensitivitySnapshots.clear();
	mAveragePoisonSensitivitySnapshots.clear();
	mAveragePunishmentSensitivitySnapshots.clear();
	mAverageMutationChanceSnapshots.clear();
	mAverageUpdatesBetweenRandomActionSnapshots.clear();

	mSelectedAgent = nullptr;
	mSelectedCell = nullptr;

	mUpdateCount = 0;
	mNutrientSpawnTickCounter = 0;
	mAgentSpawnTickCounter = 0;

	delete[] gridPixelData;
	Initialized = false;
}

void SimGrid::Update(float DeltaTime)
{
	if (mShouldAutomaticallyStop && mUpdateCount > mUpdatesToSimulate)
	{
		Beep(523, 500);
		MessageBox(nullptr, TEXT("ArtiEco"), TEXT("The Simulation Has Finished"), MB_OK);
		mIsPaused = true;
		return;
	}
	mUpdatesSinceLastHazardCheck++;
	mUpdateTimer += DeltaTime;
	float lavaSum = 0.0f;
	float poisonSum = 0.0f;
	//Allows us to slow down the simulation speed
	if (mUpdateTimer >= 1.0f / mUpdatesPerSecond)
	{
		//Reset stats variables
		mAverageLearningRate			= 0.0f;
		mAverageNutrientSensitivity		= 0.0f;
		mAverageEnergySensitivity		= 0.0f;
		mAverageHealthSensitivity		= 0.0f;
		mAverageLavaSensitivity			= 0.0f;
		mAveragePoisonSensitivity		= 0.0f;
		mAveragePunishmentSensitivity	= 0.0f;
		mAverageMutationRate			= 0.0f;
		mAverageRandomActionUpdate		= 0.0f;
		mHighestEnergyGained			= -1.0f;
		mLongestTimeAlive				= -1.0f;
		mHighestEfficiency				= -1.0f;

		mUpdateTimer = 0.0f;
		
		//Increment tick counters
		mAgentSpawnTickCounter++;
		mNutrientSpawnTickCounter++;

		//Check if new agents should be spawned
		if (mAgentSpawnTickCounter >= mTicksBetweenAgentSpawns && 
			(mAliveAgents + mNumOfAgentstoSpawn <= mMaxNumberOfAgentsForSpawn || mSpeciesCount.size() < mMinimumSpeciesDiversity))
		{
			mAgentSpawnTickCounter = 0;
			for (int i = 0; i < mNumOfAgentstoSpawn; i++)
			{
				int randPosX = utility::RandomNumber(0, mSizeX);
				int randPosY = utility::RandomNumber(0, mSizeY);
				AddAgent(randPosX, randPosY);
			}
		}

		//Check if nutrients should grow
		if (mNutrientSpawnTickCounter >= mTicksBetweenNutrientSpawns)
		{
			mNutrientSpawnTickCounter = 0;
			for (int i = 0; i < mNumOfNutrientsToSpawn; i++)
			{
				GrowPlantNutrients(utility::RandomNumber(0, mSizeX), utility::RandomNumber(0, mSizeY), utility::RandomNumber(mMinSpawnedNutrientValue, mMaxSpawnedNutrientValue));
			}
		}
		int numOfAgents = Agents.size();
		//Reset agent stats variables
		float efficiencySum = 0.0f;
		int numOfAgentsAbove1Efficiency = 0;
		mSpeciesCount.clear();
		//Update all agents
		for (auto& it : Agents)
		{
			//Update agent
			Agent* agentToUpdate = it.second;
			agentToUpdate->Update(DeltaTime);

			//Collect agent stats
			mAverageLearningRate += agentToUpdate->mAgentBrain->GetLearningRate();
			mAverageNutrientSensitivity += agentToUpdate->GetNutrientSensitivity();
			mAverageEnergySensitivity += agentToUpdate->GetEnergySensitivity();
			mAverageHealthSensitivity += agentToUpdate->GetHealthSensitivity();
			mAverageLavaSensitivity += agentToUpdate->GetLavaSensitivity();
			mAveragePoisonSensitivity += agentToUpdate->GetPoisonSensitivity();
			mAveragePunishmentSensitivity += agentToUpdate->GetPunishmentSensitivity();
			mAverageMutationRate += agentToUpdate->GetMutationChance();


			int speciesID = agentToUpdate->GetSpeciesID();
			int val = mSpeciesCount[speciesID];
			mSpeciesCount[speciesID] = val + 1;
			if (agentToUpdate->GetEfficiency() > 1.0f)
			{
				numOfAgentsAbove1Efficiency++;
			}
			efficiencySum += agentToUpdate->GetEfficiency();
			if (mUpdatesSinceLastHazardCheck >= mUpdatesBetweenHazardChecks)
			{
				poisonSum += agentToUpdate->mTimesEatenPoison;
				agentToUpdate->mTimesEatenPoison = 0;
				lavaSum += agentToUpdate->mTimesRanIntoLava;
				agentToUpdate->mTimesRanIntoLava = 0;
			}
		}
		
		if (mUpdatesSinceLastHazardCheck >= mUpdatesBetweenHazardChecks)
		{
			mUpdatesSinceLastHazardCheck = 0;
			mNumOfTimesEatenPoisonSnapshots.push_back(poisonSum / Agents.size());
			mNumOfTimesWalkedIntoLavaSnapshots.push_back(lavaSum / Agents.size());
		}
		if (numOfAgentsAbove1Efficiency > mCurrentMaxNumOfAgentsAbove1Efficiency)
		{
			mCurrentMaxNumOfAgentsAbove1Efficiency = numOfAgentsAbove1Efficiency;
		}
		if (mSpeciesCount.size() > mCurrentMaxUniqueSpeciesCount)
		{
			mCurrentMaxUniqueSpeciesCount = mSpeciesCount.size();
		}
		float sum = 0.0f;
		int currentNumOfTopAgents = 0;
		mAverageEfficiencyOfTopAgents = efficiencySum/Agents.size();
		mAverageLearningRate /= Agents.size();

		//Remove agents queued for deletion
		for (int i = 0; i < mAgentsToRemove.size(); i++)
		{
			RemoveAgent(mAgentsToRemove[i]);
		}
		mAgentsToRemove.clear();

		//Add agent data to graph arrays
		mNumOfAgentsAbove1EfficiencySnapshots	.push_back(numOfAgentsAbove1Efficiency);
		mNumOfUniqueAgentSpeciesSnapshots		.push_back(mSpeciesCount.size());
		mAvgEnergySnapshots						.push_back(mAverageEfficiencyOfTopAgents);
		mAverageNutrientSensitivitySnapshots	.push_back(mAverageNutrientSensitivity /= numOfAgents);
		mAverageEnergySensitivitySnapshots		.push_back(mAverageEnergySensitivity /= numOfAgents);
		mAverageHealthSensitivitySnapshots		.push_back(mAverageHealthSensitivity /= numOfAgents);
		mAverageLavaSensitivitySnapshots		.push_back(mAverageLavaSensitivity /= numOfAgents);
		mAveragePoisonSensitivitySnapshots		.push_back(mAveragePoisonSensitivity /= numOfAgents);
		mAveragePunishmentSensitivitySnapshots	.push_back(mAveragePunishmentSensitivity /= numOfAgents);

		mUpdateCount++;
	}
}

void SimGrid::RenderGrid()
{
	glUniform3f(mOffsetLoc, gridOffsetX, gridOffsetY, 0.0f);
	glUniform1f(mZoomLoc, mZoom);
	glUseProgram(mShaderProgramID);
	for (int i = 0; i < Cells.size(); i++)
	{
		Cells[i]->Render(mShaderProgramID, mShowAgents, mShowNutrients, mShowTerrain, mShowLava, mShowPoison);
	}
	glBindTexture(GL_TEXTURE_2D, gGridTexture);
	//glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, gGridSize, gGridSize, GL_RGB, GL_UNSIGNED_BYTE, gridPixelData);
	//glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
	glGenerateMipmap(GL_TEXTURE_2D);
	glBindVertexArray(VAO);
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
}

void SimGrid::RenderGraphs()
{
	ImGui::BeginGroup();
	ImGui::Text("Graphs");
	if (ImGui::CollapsingHeader("Average Efficiency Graph") && mAvgEnergySnapshots.size() > 0)
	{
		if (ImPlot::BeginPlot("Average Efficiency")) {
			ImPlot::SetupAxes("Time Step", "Efficiency");
			ImPlot::SetupAxesLimits(0, mAvgEnergySnapshots.size(), 0.5f, 1.5f, ImPlotCond_Always);
			ImPlot::PlotLine("Energy", &mAvgEnergySnapshots[0], mAvgEnergySnapshots.size());
			ImPlot::EndPlot();
		}
		if (ImGui::Button("Reset Energy"))
		{
			mAvgEnergySnapshots = {};
		}
	}
	if (ImGui::CollapsingHeader("Agents Above 1.0 Efficiency Graph"))
	{
		if (ImPlot::BeginPlot("Number Of Agents Above Efficiency 1.0")) {
			ImPlot::SetupAxes("Time Step", "Number");
			ImPlot::SetupAxesLimits(0, mNumOfAgentsAbove1EfficiencySnapshots.size(), 0, mCurrentMaxNumOfAgentsAbove1Efficiency + 10, ImPlotCond_Always);
			ImPlot::PlotLine("Energy", &mNumOfAgentsAbove1EfficiencySnapshots[0], mNumOfAgentsAbove1EfficiencySnapshots.size());
			ImPlot::EndPlot();
		}
	}
	if (ImGui::CollapsingHeader("Number Of Unique Species Graph"))
	{
		if (ImPlot::BeginPlot("Number Of Unique Agent Species")) {
			ImPlot::SetupAxes("Time Step", "Number");
			ImPlot::SetupAxesLimits(0, mNumOfUniqueAgentSpeciesSnapshots.size(), 0, mCurrentMaxUniqueSpeciesCount + 10, ImPlotCond_Always);
			ImPlot::PlotLine("Species", &mNumOfUniqueAgentSpeciesSnapshots[0], mNumOfUniqueAgentSpeciesSnapshots.size());
			ImPlot::EndPlot();
		}
	}
	if (ImGui::CollapsingHeader("Average Nutrient Sensitivity Graph"))
	{
		if (ImPlot::BeginPlot("Average Nutrient Sensitivity")) {
			ImPlot::SetupAxes("Time Step", "Number");
			ImPlot::SetupAxesLimits(0, mAverageNutrientSensitivitySnapshots.size(), mDefaultAgentInitData.MinNutrientSensitivity, mDefaultAgentInitData.MaxNutrientSensitivity, ImPlotCond_Always);
			ImPlot::PlotLine("Sensitivity", &mAverageNutrientSensitivitySnapshots[0], mAverageNutrientSensitivitySnapshots.size());
			ImPlot::EndPlot();
		}
	}
	if (ImGui::CollapsingHeader("Average Health Sensitivity Graph"))
	{
		if (ImPlot::BeginPlot("Average Health Sensitivity")) {
			ImPlot::SetupAxes("Time Step", "Number");
			ImPlot::SetupAxesLimits(0, mAverageHealthSensitivitySnapshots.size(), mDefaultAgentInitData.MinHealthSensitivity, mDefaultAgentInitData.MaxHealthSensitivity, ImPlotCond_Always);
			ImPlot::PlotLine("Sensitivity", &mAverageHealthSensitivitySnapshots[0], mAverageHealthSensitivitySnapshots.size());
			ImPlot::EndPlot();
		}
	}
	if (ImGui::CollapsingHeader("Average Energy Sensitivity Graph"))
	{
		if (ImPlot::BeginPlot("Average Energy Sensitivity")) {
			ImPlot::SetupAxes("Time Step", "Number");
			ImPlot::SetupAxesLimits(0, mAverageEnergySensitivitySnapshots.size(), mDefaultAgentInitData.MinEnergySensitivity, mDefaultAgentInitData.MaxEnergySensitivity, ImPlotCond_Always);
			ImPlot::PlotLine("Sensitivity", &mAverageEnergySensitivitySnapshots[0], mAverageEnergySensitivitySnapshots.size());
			ImPlot::EndPlot();
		}
	}
	if (ImGui::CollapsingHeader("Average Lava Sensitivity Graph"))
	{
		if (ImPlot::BeginPlot("Average Lava Sensitivity")) {
			ImPlot::SetupAxes("Time Step", "Number");
			ImPlot::SetupAxesLimits(0, mAverageLavaSensitivitySnapshots.size(), mDefaultAgentInitData.MinLavaSensitivity, mDefaultAgentInitData.MaxLavaSensitivity, ImPlotCond_Always);
			ImPlot::PlotLine("Sensitivity", &mAverageLavaSensitivitySnapshots[0], mAverageLavaSensitivitySnapshots.size());
			ImPlot::EndPlot();
		}
	}
	if (ImGui::CollapsingHeader("Average Poison Sensitivity Graph"))
	{
		if (ImPlot::BeginPlot("Average Poison Sensitivity")) {
			ImPlot::SetupAxes("Time Step", "Number");
			ImPlot::SetupAxesLimits(0, mAveragePoisonSensitivitySnapshots.size(), mDefaultAgentInitData.MinPoisonSensitivity, mDefaultAgentInitData.MaxPoisonSensitivity, ImPlotCond_Always);
			ImPlot::PlotLine("Sensitivity", &mAveragePoisonSensitivitySnapshots[0], mAveragePoisonSensitivitySnapshots.size());
			ImPlot::EndPlot();
		}
	}
	if (ImGui::CollapsingHeader("Average Punishment Sensitivity Graph"))
	{
		if (ImPlot::BeginPlot("Average Punishment Sensitivity")) {
			ImPlot::SetupAxes("Time Step", "Number");
			ImPlot::SetupAxesLimits(0, mAveragePunishmentSensitivitySnapshots.size(), mDefaultAgentInitData.MinPunishmentSensitivity, mDefaultAgentInitData.MaxPunishmentSensitivity, ImPlotCond_Always);
			ImPlot::PlotLine("Sensitivity", &mAveragePunishmentSensitivitySnapshots[0], mAveragePunishmentSensitivitySnapshots.size());
			ImPlot::EndPlot();
		}
	}
	/*if (ImGui::CollapsingHeader("Number Of Lava Incidents Graph"))
	{
		if (ImPlot::BeginPlot("Number Of Times Walked Into Lava")) {
			ImPlot::SetupAxes("Time Step", "Number");
			ImPlot::SetupAxesLimits(0, mNumOfTimesWalkedIntoLava.size(), 0, mUpdatesBetweenHazardChecks / 2, ImPlotCond_Always);
			ImPlot::PlotLine("Species", &mNumOfTimesWalkedIntoLava[0], mNumOfTimesWalkedIntoLava.size());
			ImPlot::EndPlot();
		}
	}
	if (ImGui::CollapsingHeader("Number Of Poison Incidents Graph"))
	{
		if (ImPlot::BeginPlot("Number Of Times Eaten Poison")) {
			ImPlot::SetupAxes("Time Step", "Number");
			ImPlot::SetupAxesLimits(0, mNumOfTimesEatenPoison.size(), 0, mUpdatesBetweenHazardChecks / 2, ImPlotCond_Always);
			ImPlot::PlotLine("Species", &mNumOfTimesEatenPoison[0], mNumOfTimesEatenPoison.size());
			ImPlot::EndPlot();
		}
	}*/
	ImGui::Text("--------------------");
	ImGui::EndGroup();
}

void SimGrid::RenderNeuralNetwork()
{
	//Render network
	ImGui::Begin("Agent Network");
	ImDrawList* draw_list = ImGui::GetWindowDrawList();
	data::vector<int> order = mSelectedAgent->GetBrainLayerOrder();
	const ImVec2 p = ImGui::GetCursorScreenPos();
	int circleSize = 16.0f;
	int x = p.x + 4.0f;
	int y = p.y + 4.0f;
	int xSpacing = 200.0f;
	int ySpacing = 20.0f;
	float centerLine = 0.0f;
	std::vector<int> neuronsToLink;
	int maxNumOfNeurons = 0;
	float maxWeightVal = 0.0f;
	float maxLineWidth = 5.0f;
	for (int i = 0; i < order.size(); i++)
	{
		Layer currentLayer = mSelectedAgent->GetBrainLayer(order[i]);
		if (currentLayer.NeuronIDs.size() > maxNumOfNeurons)
		{
			maxNumOfNeurons = currentLayer.NeuronIDs.size();
		}
	}
	centerLine = maxNumOfNeurons * circleSize + ((maxNumOfNeurons - 1) * ySpacing);
	centerLine *= 0.5f;
	for (int i = 0; i < order.size(); i++)
	{
		Layer currentLayer = mSelectedAgent->GetBrainLayer(order[i]);
		int numOfNeurons = currentLayer.NeuronIDs.size();
		y = p.y + (centerLine - (numOfNeurons * circleSize + ((numOfNeurons - 1) * ySpacing)) / 2.0f);
		for (int j = 0; j < numOfNeurons; j++)
		{
			Neuron* currentNeuron = mSelectedAgent->mAgentBrain->GetNeuron(currentLayer.NeuronIDs[j]);
			currentNeuron->disPosX = x + circleSize * 0.5f;
			currentNeuron->disPosY = y + circleSize * 0.5f;
			for (int j = 0; j < currentNeuron->OutputLinks.size(); j++)
			{
				Link currentLink = mSelectedAgent->mAgentBrain->GetLinkData(currentNeuron->OutputLinks[j]);
				if (abs(currentLink.Weight) > maxWeightVal)
				{
					maxWeightVal = abs(currentLink.Weight);
				}
			}
			neuronsToLink.push_back(currentNeuron->ID);
			y += ySpacing + circleSize;
		}
		x += xSpacing + circleSize;
	}


	for (int i = 0; i < neuronsToLink.size(); i++)
	{
		Neuron* currentNeuron = mSelectedAgent->mAgentBrain->GetNeuron(neuronsToLink[i]);
		for (int j = 0; j < currentNeuron->OutputLinks.size(); j++)
		{
			Link currentLink = mSelectedAgent->mAgentBrain->GetLinkData(currentNeuron->OutputLinks[j]);
			float percent = abs(currentLink.Weight) / maxWeightVal;
			if (percent > 1.0f)
			{
				bool wut = false;
			}
			float thickness = maxLineWidth * percent;
			ImColor lineColor = ImColor(0.0f, 1.0f, 1.0f);
			if (currentLink.Weight < 0.0f)
			{
				lineColor = ImColor(1.0f, 0.0f, 0.5f);
				thickness = abs(thickness);
			}
			if (currentLink.Weight == 0.0f)
			{
				lineColor = ImColor(0.1f, 0.1f, 0.1f);
				thickness = 1.0f;
			}
			Neuron* targetNeuron = mSelectedAgent->mAgentBrain->GetNeuron(currentLink.OutputNeuron);
			draw_list->AddLine(ImVec2(currentNeuron->disPosX, currentNeuron->disPosY), ImVec2(targetNeuron->disPosX, targetNeuron->disPosY), lineColor, thickness);
		}
	}

	for (int i = 0; i < order.size(); i++)
	{
		Layer currentLayer = mSelectedAgent->GetBrainLayer(order[i]);
		int numOfNeurons = currentLayer.NeuronIDs.size();
		for (int j = 0; j < numOfNeurons; j++)
		{
			Neuron* currentNeuron = mSelectedAgent->mAgentBrain->GetNeuron(currentLayer.NeuronIDs[j]);
			ImColor neuronColor = ImColor(0.0f, 1.0f, 1.0f);
			if (i == order.size() - 1)
			{
				float percent = (currentNeuron->Activation / mSelectedAgent->mAgentBrain->mCurrentOverallActivationValue);
				//percent *= 4;
				
				neuronColor = ImColor(0.0f, percent, percent);
				percent *= 100.0f;
				char str[16];
				snprintf(str, sizeof(str), "%.2f", percent);
				draw_list->AddText(ImVec2(currentNeuron->disPosX + 10.0f, currentNeuron->disPosY), ImColor(1.0f, 1.0f, 1.0f), str);
			}
			else
			{
				neuronColor = ImColor(0.0f, currentNeuron->Activation, currentNeuron->Activation);
			}
			draw_list->AddCircleFilled(ImVec2(currentNeuron->disPosX, currentNeuron->disPosY), circleSize * 0.5f, neuronColor, 0);
			char charVal[10];
			snprintf(charVal, sizeof charVal, "%.2f", currentNeuron->Activation);
		}
	}

	ImGui::End();
}

void SimGrid::PrintGridControls()
{
	if (!Initialized)
	{
		ImGui::Text("Initialization Variables");
		ImGui::InputInt("Seed", &gInitSeed);
		int size = mSizeX;
		ImGui::InputInt("Grid Size", &size);
		mSizeX = size;
		mSizeY = size;
		ImGui::InputFloat("Initial Solid Density", &mSolidDensity);
		ImGui::InputFloat("Initial Nutrient Density", &mInitialNutrientDensity);
		ImGui::InputFloat("Initial Lava Density", &mLavaDensity);
		ImGui::InputFloat("Initial Poison Density", &mPoisonDensity);
	}

	ImGui::Text("Grid Variables");
	ImGui::Checkbox("Should Simulation Stop?", &mShouldAutomaticallyStop);
	if (mShouldAutomaticallyStop)
	{
		ImGui::InputInt("Updates to simulate", &mUpdatesToSimulate);
	}
	ImGui::Checkbox("Pause Simulation", &mIsPaused);
	ImGui::Checkbox("Should Agents Learn", &mShouldAgentsLearn);
	ImGui::InputInt("Updates Per Second", &mUpdatesPerSecond);

	ImGui::Text("Agent Spawn Variables");
	ImGui::InputInt("Max Random Agents To Spawn", &mMaxNumberOfAgentsForSpawn);
	ImGui::InputInt("Updates Between Agent Spawns", &mTicksBetweenAgentSpawns);
	ImGui::InputInt("Number Of Agents To Spawn", &mNumOfAgentstoSpawn);
	ImGui::InputInt("Minimum Species Count", &mMinimumSpeciesDiversity);
	ImGui::Checkbox("Randomize agent values", &mRandomizeAgentValues);
	if(mRandomizeAgentValues)
	{
		ImGui::InputFloat2("Nutrient Sensitivity Range", &mDefaultAgentInitData.MinNutrientSensitivity);
		ImGui::InputFloat2("Energy Sensitivity Range", &mDefaultAgentInitData.MinEnergySensitivity);
		ImGui::InputFloat2("Health Sensitivity Range", &mDefaultAgentInitData.MinHealthSensitivity);
		ImGui::InputFloat2("Lava Sensitivity Range", &mDefaultAgentInitData.MinLavaSensitivity);
		ImGui::InputFloat2("Poison Sensitivity Range", &mDefaultAgentInitData.MinPoisonSensitivity);
		ImGui::InputFloat2("Punishment Sensitivity Range", &mDefaultAgentInitData.MinPunishmentSensitivity);
		ImGui::InputFloat2("Mutation Chance Range", &mDefaultAgentInitData.MinMutationChance);
		ImGui::InputFloat2("Network Connectivity Range", &mDefaultAgentInitData.MinNetworkConnectivity);
		ImGui::InputInt2("Random Event Update Number Range", &mDefaultAgentInitData.MinUpdatesForRandomAction);
	}


	ImGui::Text("Nutrient Spawn Variables");
	ImGui::InputInt("Updates Between Nutrient Spawns", &mTicksBetweenNutrientSpawns);
	ImGui::InputInt("Number Of Nutrient Spawns", &mNumOfNutrientsToSpawn);
	ImGui::InputFloat("Max Nutrient Amount Per Spawn", &mMaxSpawnedNutrientValue);
	ImGui::InputFloat("Min Nutrient Amount Per Spawn", &mMinSpawnedNutrientValue);
	
	ImGui::Text("Visibility Variables");
	ImGui::Checkbox("Show Nutrients", &mShowNutrients);
	ImGui::Checkbox("Show Agents", &mShowAgents);
	ImGui::Checkbox("Show Terrain", &mShowTerrain);
	ImGui::Checkbox("Show Lava", &mShowLava);
	ImGui::Checkbox("Show Poison", &mShowPoison);
}

void SimGrid::PrintCellData()
{
	CellData data = mSelectedCell->GetData();

	ImGui::Text("----CELL DATA----");
	int posX, posY;
	mSelectedCell->GetCellPos(posX, posY);
	ImGui::Text("Position: (%i, %i)", posX, posY);
	ImGui::Text("Nutrients: %.2f", data.mPlantNutrients);

	{
		static char brainLoadName[128] = "";
		ImGui::InputText("Brain Load", brainLoadName, IM_ARRAYSIZE(brainLoadName));
		if (ImGui::Button("Load Agent Brain"))
		{
			if (!mSelectedCell->HasAgent())
			{
				CreateNewAgentWithBrain(brainLoadName);
			}
		}
	}
	if (data.mOccupyingAgent != nullptr)
	{
		ImGui::Text("Occupying Agent ID: %i", mSelectedCell->GetData().mOccupyingAgent->GetID());
	}
	else
	{
		ImGui::Text("Occupying Agent ID: NONE");
	}
	mSelectedCell->DisplayCellControls();
	if (ImGui::Button("Deselect Cell"))
	{
		mSelectedCell = nullptr;
	}
}

void SimGrid::PrintAgentData()
{
	
	Brain* currentBrain = mSelectedAgent->mAgentBrain.get();
	ImGui::Text("Agent Controls");
	if (ImGui::Button("Delete Agent"))
	{
		mSelectedAgent->Die();
	}
	ImGui::InputFloat("Learning Rate", &currentBrain->mLearningSpeed);
	ImGui::InputInt("Future Depth", &currentBrain->mFutureDepth);
	ImGui::InputFloat("Learning Discount Factor", &currentBrain->mDiscountFactor);
	ImGui::Text("----AGENT DATA----");
	ImGui::Text("Agent ID: %i", mSelectedAgent->GetID());
	utility::Vector2i pos = mSelectedAgent->GetPosition();
	ImGui::Text("Energy: %.2f", mSelectedAgent->GetEnergy());
	ImGui::Text("Energy Efficiency: %.2f", mSelectedAgent->GetEfficiency());
	ImGui::Text("Agent Position: (%i, %i)", pos.X, pos.Y);
	ImGui::Text("Learning Rate: %.2f", currentBrain->GetLearningRate());
	ImGui::Text("Objective Value: %.2f", currentBrain->mObjectiveFunction);
	{
		static char brainSaveName[128] = "";
		ImGui::InputText("Brain Save", brainSaveName, IM_ARRAYSIZE(brainSaveName));
		if (ImGui::Button("Save Agent Brain"))
		{
			mSelectedAgent->mAgentBrain.get()->SaveBrain(brainSaveName);
		}
	}

	if (ImGui::Button("Deselect Agent"))
	{
		mSelectedAgent = nullptr;
	}
}

void SimGrid::AddCell(float StartPosX, float StartPosY, int PosX, int PosY)
{
	float cellPositionX = mCellSize * PosX + StartPosX;
	float cellPositionY = mCellSize * PosY + StartPosY;

	std::array<float, 16> VertexPositions =
	{
		cellPositionX,				cellPositionY,				0.0f,	//Top left
		cellPositionX + mCellSize,	cellPositionY,				0.0f,	//Top right
		cellPositionX,				cellPositionY + mCellSize,	0.0f,	//Bottom left
		cellPositionX + mCellSize,	cellPositionY + mCellSize,	0.0f	//Bottom right
	};

	Cells.push_back(new Cell({VertexPositions}, PosX, PosY, this));
}

void SimGrid::AddNutrients(int PosX, int PosY, float Amount)
{
	GetCell(PosX, PosY)->AddPlantNutrients(Amount);
}

void SimGrid::AddGroundNutrients(int PosX, int PosY, float Amount)
{
	GetCell(PosX, PosY)->AddGroundNutrients(Amount);
}

void SimGrid::GrowPlantNutrients(int PosX, int PosY, float Amount)
{
	GetCell(PosX, PosY)->ConvertGroundToPlantNutrients(Amount);
}

float SimGrid::GetNutrientsSurroundingCell(int PosX, int PosY)
{
	float val = 0.0f;
	int mask[8][2] =
	{
		{PosX + -1, PosY + 1}, {PosX + 0, PosY + 1}, {PosX + 1, PosY + 1},
		{PosX + -1, PosY + 0},                       {PosX + 1, PosY + 0},
		{PosX + -1,PosY + -1}, {PosX + 0, PosY + -1},{PosX + 1, PosY + -1},
	};
	for (int i = 0; i < 8; i++)
	{
		val += GetCell(mask[i][0], mask[i][1])->GetData().mPlantNutrients;
	}
	return val;
}

int SimGrid::GetLavaSurroundingCell(int PosX, int PosY)
{
	int val = 0;
	int mask[8][2] =
	{
		{PosX + -1, PosY + 1}, {PosX + 0, PosY + 1}, {PosX + 1, PosY + 1},
		{PosX + -1, PosY + 0},                       {PosX + 1, PosY + 0},
		{PosX + -1,PosY + -1}, {PosX + 0, PosY + -1},{PosX + 1, PosY + -1},
	};
	for (int i = 0; i < 8; i++)
	{
		val += GetCell(mask[i][0], mask[i][1])->GetData().mIsLava;
	}
	return val;
}

int SimGrid::GetPoisonSurroundingCell(int PosX, int PosY)
{
	int val = 0;
	int mask[8][2] =
	{
		{PosX + -1, PosY + 1}, {PosX + 0, PosY + 1}, {PosX + 1, PosY + 1},
		{PosX + -1, PosY + 0},                       {PosX + 1, PosY + 0},
		{PosX + -1,PosY + -1}, {PosX + 0, PosY + -1},{PosX + 1, PosY + -1},
	};
	for (int i = 0; i < 8; i++)
	{
		val += GetCell(mask[i][0], mask[i][1])->GetData().mIsPoisoned;
	}
	return val;
}

Agent* SimGrid::AddAgent(int PosX, int PosY)
{
	if (PosX >= 0 && PosX < mSizeX &&
		PosY >= 0 && PosY < mSizeY &&
		GetAgentAtCell(PosX, PosY) == nullptr &&
		!GetCell(PosX, PosY)->GetData().mSolid)
	{
		AgentInitData data = mDefaultAgentInitData;
		float connectivity = 1.0f;
		if (mRandomizeAgentValues)
		{
			connectivity = utility::RandomNumber(data.MinNetworkConnectivity, data.MaxNetworkConnectivity);

		}
		Agent* agentToAdd = new Agent(PosX, PosY, mAgentIDCount, mSpeciesIDCount, connectivity, this);
		agentToAdd->mAgentColour = { utility::RandomNumber(0.0f, 1.0f), utility::RandomNumber(0.0f, 1.0f), utility::RandomNumber(0.0f, 1.0f) };
		//Randomized variables
		if (mRandomizeAgentValues)
		{
			agentToAdd->SetNutrientSensitivity(utility::RandomNumber(data.MinNutrientSensitivity, data.MaxNutrientSensitivity));
			agentToAdd->SetEnergySensitivity(utility::RandomNumber(data.MinEnergySensitivity, data.MaxEnergySensitivity));
			agentToAdd->SetHealthSensitivity(utility::RandomNumber(data.MinHealthSensitivity, data.MaxHealthSensitivity));
			agentToAdd->SetLavaSensitivity(utility::RandomNumber(data.MinLavaSensitivity, data.MaxLavaSensitivity));
			agentToAdd->SetPoisonSensitivity(utility::RandomNumber(data.MinPoisonSensitivity, data.MaxPoisonSensitivity));
			agentToAdd->SetPunishmentSensitivity(utility::RandomNumber(data.MinPunishmentSensitivity, data.MaxPunishmentSensitivity));
			agentToAdd->SetMutationChance(utility::RandomNumber(data.MinMutationChance, data.MaxMutationChance));
			agentToAdd->mAgentBrain.get()->SetRandomActionUpdateCount(utility::RandomNumber(data.MinUpdatesForRandomAction, data.MaxUpdatesForRandomAction));
		}
		Agents.insert(std::pair<int, Agent*>(mAgentIDCount, agentToAdd));
		mAliveAgents++;
		mAgentIDCount++;
		mSpeciesIDCount++;
		return agentToAdd;
	}
	return nullptr;
}

Agent* SimGrid::AddAgent(Agent* Parent, int PosX, int PosY)
{
	if (PosX >= 0 && PosX < mSizeX &&
		PosY >= 0 && PosY < mSizeY &&
		GetAgentAtCell(PosX, PosY) == nullptr &&
		!GetCell(PosX, PosY)->GetData().mSolid)
	{
		Agent* agentToAdd = new Agent(Parent, PosX, PosY, mAgentIDCount, this);
		Agents.insert(std::pair<int, Agent*>(mAgentIDCount, agentToAdd));
		mAliveAgents++;
		mAgentIDCount++;
		return agentToAdd;
	}
	return nullptr;
}

Agent* SimGrid::AddAgentAtRandomPos()
{
	return AddAgent(utility::RandomNumber(0, mSizeX), utility::RandomNumber(0, mSizeY));
}

Agent* SimGrid::GetAgentAtCell(int PosX, int PosY)
{
	Agent* foundAgent = nullptr;
	GetCell(PosX, PosY)->GetOccupyingAgent(foundAgent);
	return foundAgent;
}

void SimGrid::RemoveAgent(Agent* AgentToRemove)
{
	Cell* cellContainingAgent = AgentToRemove->GetCell();
	if (cellContainingAgent != nullptr)
	{
		if (mSelectedAgent != nullptr && mSelectedAgent->GetID() == AgentToRemove->GetID())
		{
			mSelectedAgent = nullptr;
		}
		cellContainingAgent->ClearAgent();
		Agents.erase(AgentToRemove->GetID());
		mAliveAgents--;
		delete(AgentToRemove);
	}
}

Cell* SimGrid::GetCell(int X, int Y)
{
	if (mWrapAround)
	{
		if (X < 0)
		{
			X = mSizeX - 1;
		}
		if (X >= mSizeX)
		{
			X = 0;
		}
		if (Y < 0)
		{
			Y = mSizeY - 1;
		}
		if (Y >= mSizeY)
		{
			Y = 0;
		}
	}

	if (X >= 0 && X < mSizeX && Y >=0 && Y < mSizeY)
	{
		int CellIndex = ((Y * mSizeX) + X);

		if (CellIndex < Cells.size() && CellIndex >= 0)
		{
			return Cells[CellIndex];
		}
	}
	return nullptr;
}

Agent* SimGrid::CreateNewAgentWithBrain(std::string FileName)
{
	Brain* loadedBrain = LoadBrain(FileName);
	if (loadedBrain != nullptr)
	{
		int posX, posY;
		mSelectedCell->GetCellPos(posX, posY);
		Agent* agentToAdd = new Agent(posX, posY, mAgentIDCount, mSpeciesIDCount, this, loadedBrain);
		agentToAdd->mAgentColour = { utility::RandomNumber(0.0f, 1.0f), utility::RandomNumber(0.0f, 1.0f), utility::RandomNumber(0.0f, 1.0f) };
		Agents.insert(std::pair<int, Agent*>(mAgentIDCount, agentToAdd));
		mAliveAgents++;
		mAgentIDCount++;
		mSpeciesIDCount++;
		mSelectedAgent = agentToAdd;
		return agentToAdd;
	}
	return nullptr;
}

Brain* SimGrid::LoadBrain(std::string FileName)
{
	constexpr auto const CISMODE = cista::mode::WITH_VERSION |
		cista::mode::WITH_INTEGRITY |
		cista::mode::DEEP_CHECK;
	if (std::filesystem::exists("Brains/" + FileName + ".txt"))
	{
		std::ifstream file("Brains/" + FileName + ".txt", std::ios::in | std::ios::binary);
		std::vector<uint8_t> loadedData((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
		try {
			auto loadedBrain = cista::deserialize<Brain, CISMODE>(loadedData);
			Brain* newBrain = new Brain();
			newBrain->SetupBrainFromLoadedBrain(loadedBrain);
			return newBrain;
		}
		catch (exception e)
		{

		}
	}
	return nullptr;
}