#pragma once

#include "../core/Graph.h"
#include "../core/Web.h"

#include <string>
#include <vector>

struct AllocResult {
    bool feasible = false;
    int registersUsed = 0;
    std::vector<Web> webs;
    std::vector<int> reg;
    std::vector<bool> spilled;
    std::string note;
};

namespace Allocator {

AllocResult basic(const std::vector<Web> &webs, int maxRegs);
AllocResult spilling(const std::vector<Web> &webs, int maxRegs, int spillBudget);
AllocResult splitting(const std::vector<Web> &webs, int maxRegs, int splitBudget);
AllocResult freeStrategy(const std::vector<Web> &webs, int maxRegs);

}
