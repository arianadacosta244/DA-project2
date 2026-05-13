#pragma once

#include "../algo/Allocator.h"

#include <ostream>
#include <string>

namespace OutputWriter {

void write(std::ostream &os, const AllocResult &result);
void writeToFile(const std::string &filename, const AllocResult &result);

}
