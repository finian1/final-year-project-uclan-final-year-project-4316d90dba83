#pragma once
class Brain;
class LearningAlgorithm
{
public:
	virtual void AdjustWeights(Brain& inputBrain);
};

