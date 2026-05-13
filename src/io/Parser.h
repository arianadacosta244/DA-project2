#pragma once

#include "../core/Web.h"

#include <string>
#include <vector>

struct AllocatorConfig {
    int registers = 0;
    std::string algorithm;
    int parameter = 0;
};

namespace Parser {

std::vector<LiveRange> parseRanges(const std::string &filename);
AllocatorConfig parseConfig(const std::string &filename);

}
