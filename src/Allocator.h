/**
 * @file Allocator.h
 * @brief Register-allocation strategies (basic, spilling, splitting, free).
 */
#pragma once

#include "Graph.h"
#include "Web.h"

#include <string>
#include <vector>

/**
 * @brief Output of a register-allocation attempt.
 *
 * Indices into @c reg / @c spilled correspond to the @c id of each web in
 * @ref webs. @c reg[i] holds the assigned color (= register index) in
 * @c [0, registersUsed) or @c -1 if the web was spilled / unassigned.
 *
 * When @c feasible is @c false the meaning is: "no valid assignment could
 * be produced within the budget of the requested algorithm" — in that case
 * all webs are reported as memory in the output file (per spec Figure 12).
 */
struct AllocResult {
    bool feasible = false;
    int registersUsed = 0;
    std::vector<Web> webs;           ///< possibly modified webs (after splitting)
    std::vector<int> reg;            ///< color of each web (or -1 for memory)
    std::vector<bool> spilled;       ///< true if the web is in memory
    std::string note;                ///< human-readable summary of how we got here
};

namespace Allocator {

/**
 * @brief T2.1 — basic Chaitin-style greedy coloring without spilling.
 *
 * Searches for the minimum K in @c [1, maxRegs] that lets the standard
 * simplify-and-pop scheme empty the graph. Fails if even K = maxRegs is
 * insufficient.
 *
 * @complexity O(maxRegs · |V| · (|V| + |E|))
 */
AllocResult basic(const std::vector<Web> &webs, int maxRegs);

/**
 * @brief T2.2 — coloring + web spilling.
 *
 * Starts at 0 spilled webs (= basic) and grows up to @p spillBudget. At each
 * stuck point the highest-active-degree web is chosen as the spill candidate
 * (Chaitin's heuristic): the web most likely to cause future interferences.
 *
 * @complexity O(spillBudget · maxRegs · |V| · (|V| + |E|))
 */
AllocResult spilling(const std::vector<Web> &webs, int maxRegs, int spillBudget);

/**
 * @brief T2.3 — coloring + web splitting.
 *
 * If basic coloring fails, repeatedly pick a high-degree web with multiple
 * definition points and split it at every '+', reducing interference. Up to
 * @p splitBudget splits are performed. If no multi-definition web is left
 * but we are still stuck, splits at the midpoint of the highest-degree web
 * as a fallback. Splits are applied to a working copy of @p webs and the
 * (possibly larger) web set is returned in @c result.webs.
 *
 * @complexity O(splitBudget · maxRegs · |V| · (|V| + |E|) + splitBudget · |V|²)
 */
AllocResult splitting(const std::vector<Web> &webs, int maxRegs, int splitBudget);

/**
 * @brief T2.4 — free strategy.
 *
 * Uses DSatur coloring (Brélaz, 1979): always picks the uncolored vertex with
 * highest saturation degree (most distinct colors among colored neighbours),
 * breaking ties by raw degree, and assigns it the smallest legal color.
 * For interference graphs this typically reaches the chromatic number on
 * sparse / planar-like inputs faster than the simplify-and-pop pass, and
 * gracefully degrades by spilling the still-uncolored vertices.
 *
 * @complexity O(|V|² + |V| · |E|)
 */
AllocResult freeStrategy(const std::vector<Web> &webs, int maxRegs);

} // namespace Allocator
