#pragma once

#include "../core/Graph.h"
#include "../core/Web.h"

#include <memory>
#include <vector>

namespace InterferenceBuilder {

std::unique_ptr<Graph<int>> build(const std::vector<Web> &webs);

}
