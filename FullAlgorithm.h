#pragma once
#include "LearningAlgorithm.h"
class Brain;
class FullAlgorithm :
    public LearningAlgorithm
{
public:
    void AdjustWeights(Brain& inputBrain) override;
};

