/**
 * @file OutputWriter.h
 * @brief Emit the spec's output format (Figures 11 / 12) to a file and/or stream.
 */
#pragma once

#include "Allocator.h"

#include <ostream>
#include <string>

namespace OutputWriter {

/**
 * @brief Write @p result in the format from the project's Figure 11/12 spec.
 *
 * Layout:
 * @code
 * # webs section
 * webs: <N>
 * web0: <line list>
 * web1: <line list>
 * ...
 * # registers section
 * registers: <K>
 * r0: webX
 * r0: webY
 * r1: webZ
 * # plus one "M: webX" line per spilled web (only if infeasible OR partial spilling)
 * @endcode
 */
void write(std::ostream &os, const AllocResult &result);

/// Convenience: open @p filename for writing and call the stream variant.
void writeToFile(const std::string &filename, const AllocResult &result);

} // namespace OutputWriter
