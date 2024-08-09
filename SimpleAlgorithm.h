#pragma once
#include "LearningAlgorithm.h"

class Brain;
class SimpleAlgorithm :
    public LearningAlgorithm
{
public:
    void AdjustWeights(Brain& brain) override;
};

