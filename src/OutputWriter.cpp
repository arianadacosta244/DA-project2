/**
 * @file OutputWriter.cpp
 */
#include "OutputWriter.h"

#include <fstream>
#include <stdexcept>

void OutputWriter::write(std::ostream &os, const AllocResult &result) {
    os << "# Total number of webs followed by the listing of the program points of each one\n";
    os << "# program points in each web are sorted in ascending order\n";
    os << "webs: " << result.webs.size() << '\n';
    for (const auto &w : result.webs)
        os << "web" << w.id << ": " << w.serialize() << '\n';

    os << "# Total number of registers used, followed by assignment to webs\n";
    if (!result.feasible) {
        os << "registers: 0\n";
        for (const auto &w : result.webs)
            os << "M: web" << w.id << '\n';
        return;
    }

    os << "registers: " << result.registersUsed << '\n';
    // Group: for each register r in [0, K), emit lines "r<r>: web<id>" for every
    // web carrying that color (sorted by web id for determinism).
    for (int r = 0; r < result.registersUsed; ++r) {
        for (std::size_t i = 0; i < result.webs.size(); ++i) {
            if (!result.spilled[i] && result.reg[i] == r)
                os << "r" << r << ": web" << result.webs[i].id << '\n';
        }
    }
    // Then memory-resident (spilled) webs, if any.
    for (std::size_t i = 0; i < result.webs.size(); ++i) {
        if (result.spilled[i])
            os << "M: web" << result.webs[i].id << '\n';
    }
}

void OutputWriter::writeToFile(const std::string &filename, const AllocResult &result) {
    std::ofstream out(filename);
    if (!out) throw std::runtime_error("Cannot open output file: " + filename);
    write(out, result);
}
