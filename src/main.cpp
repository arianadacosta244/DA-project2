/**
 * @file main.cpp
 * @brief Entry point. Dispatches between batch and interactive modes.
 *
 * Batch mode:
 * @code
 *   regalloc -b <ranges.txt> <registers.txt> <allocation.txt>
 * @endcode
 *
 * Interactive mode (no flag): launches the menu in @ref Menu::run.
 */
#include "Allocator.h"
#include "Menu.h"
#include "OutputWriter.h"
#include "Parser.h"
#include "Web.h"

#include <cstdlib>
#include <iostream>
#include <string>

namespace {

int runBatch(const std::string &rangesFile,
             const std::string &configFile,
             const std::string &outputFile) {
    try {
        const auto ranges = Parser::parseRanges(rangesFile);
        const auto cfg    = Parser::parseConfig(configFile);
        const auto webs   = buildWebs(ranges);

        AllocResult res;
        if      (cfg.algorithm == "basic")     res = Allocator::basic(webs, cfg.registers);
        else if (cfg.algorithm == "spilling")  res = Allocator::spilling(webs, cfg.registers, cfg.parameter);
        else if (cfg.algorithm == "splitting") res = Allocator::splitting(webs, cfg.registers, cfg.parameter);
        else if (cfg.algorithm == "free")      res = Allocator::freeStrategy(webs, cfg.registers);
        else {
            std::cerr << "Unknown algorithm: " << cfg.algorithm << '\n';
            return 2;
        }

        std::cerr << res.note << '\n';
        if (!res.feasible)
            std::cerr << "WARNING: register assignment infeasible — all webs spilled to memory.\n";

        OutputWriter::writeToFile(outputFile, res);
        return res.feasible ? 0 : 1;
    } catch (const std::exception &e) {
        std::cerr << "Error: " << e.what() << '\n';
        return 2;
    }
}

void printUsage(const char *prog) {
    std::cerr << "Usage:\n"
              << "  " << prog << "                                  # interactive menu\n"
              << "  " << prog << " -b <ranges> <registers> <output> # batch mode\n";
}

} // namespace

int main(int argc, char **argv) {
    if (argc == 1) return Menu::run();

    const std::string a1 = argv[1];
    if (a1 == "-h" || a1 == "--help") { printUsage(argv[0]); return 0; }
    if (a1 == "-b") {
        if (argc != 5) { printUsage(argv[0]); return 2; }
        return runBatch(argv[2], argv[3], argv[4]);
    }
    printUsage(argv[0]);
    return 2;
}
