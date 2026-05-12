/**
 * @file InterferenceBuilder.h
 * @brief Build the interference graph from a list of webs.
 */
#pragma once

#include "Graph.h"
#include "Web.h"

#include <memory>
#include <vector>

namespace InterferenceBuilder {

/**
 * @brief Construct the interference graph for @p webs.
 *
 * Each web becomes a vertex whose payload is its @c id (the same index as in
 * the @p webs vector). Two vertices are connected by an undirected edge iff
 * @ref Web::interferesWith returns @c true for the two webs.
 *
 * @complexity O(|webs|² · L) where L is the average number of program points
 *             per web (the interference test is linear in the two webs).
 */
std::unique_ptr<Graph<int>> build(const std::vector<Web> &webs);

} // namespace InterferenceBuilder
