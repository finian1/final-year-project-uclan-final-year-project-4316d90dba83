#pragma once
#include "Actions.h"
#include "Inputs.h"
#include <vector>
#include <random>
#include <unordered_map>

static std::mt19937 gRandomNumberGen;
static int gInitSeed = 1;
//static const int gGridSize = 1000;
//static const float gCellSize = 2.0f / gGridSize;
static int gTerrainSmoothing = 2;
static float gMaxTerrainHeight = 1.0f;

static unsigned int gGridTexture;
static Actions gActions;
static Inputs gInputs;

static double gCursorPosX = 0.0;
static int gSelectedCellX = 0;
static double gCursorPosY = 0.0;
static int gSelectedCellY = 0;

enum ACTIONTYPE
{
	NOACTION,
	MOVEFORWARD,
	TURNRIGHT,
	TURNLEFT,
	EAT,
	ATTACK
};

static const std::unordered_map<ACTIONTYPE, void (*)(ActionData, Agent*)> gActionMap =
{
	{NOACTION, nullptr},
	{MOVEFORWARD, gActions.MoveForward},
	{TURNRIGHT, gActions.TurnRight},
	{TURNLEFT, gActions.TurnLeft},
	{EAT, gActions.Eat},
	{ATTACK, gActions.Attack}
};

enum INPUTTYPE
{
	NOINPUT,
	NUTRIENTSENSE,
	AGENTSENSE,
	SOLIDSENSE,
	HUNGERSENSE,
	HEIGHTSENSE,
	LAVASENSE,
	POISONSENSE,
	POSITIONSENSEX,
	POSITIONSENSEY
};

static const std::unordered_map<INPUTTYPE, float(*)(InputData, Agent*)> gInputMap =
{
	{NOINPUT, nullptr},
	{NUTRIENTSENSE, gInputs.NutrientSense},
	{AGENTSENSE, gInputs.AgentSense},
	{SOLIDSENSE, gInputs.SolidSense},
	{HUNGERSENSE, gInputs.HungerSense},
	{HEIGHTSENSE, gInputs.HeightSense},
	{LAVASENSE, gInputs.LavaSense},
	{POISONSENSE, gInputs.PoisonSense},
	{POSITIONSENSEX, gInputs.PositionSenseX},
	{POSITIONSENSEY, gInputs.PositionSenseY}
};