/**
 * @file Parser.h
 * @brief Text parsers for the two input files (live ranges + register config).
 */
#pragma once

#include "Web.h"

#include <string>
#include <vector>

/**
 * @brief Configuration loaded from the registers/algorithm file.
 *
 * Example file:
 * @code
 * # comment
 * registers: 3
 * algorithm: spilling, 2
 * @endcode
 */
struct AllocatorConfig {
    int registers = 0;        ///< maximum number of physical registers available
    std::string algorithm;    ///< "basic" | "spilling" | "splitting" | "free"
    int parameter = 0;        ///< K for spilling/splitting; 0 otherwise
};

/**
 * @brief Free functions used to parse the two text input files.
 *
 * Both parsers skip blank lines and @c '#' comment lines. They throw
 * std::runtime_error on malformed input.
 */
namespace Parser {

/**
 * @brief Read live ranges from a file.
 *
 * Each non-comment, non-blank line has the form
 * @code
 *   <varname>: <line>[+|-], <line>[+|-], ...
 * @endcode
 * Multiple lines may share the same @c varname, each describing a separate range.
 *
 * @complexity O(F · L) where F is file size and L is the average line length.
 */
std::vector<LiveRange> parseRanges(const std::string &filename);

/**
 * @brief Read the register/algorithm configuration file.
 * @complexity O(F)
 */
AllocatorConfig parseConfig(const std::string &filename);

} // namespace Parser
