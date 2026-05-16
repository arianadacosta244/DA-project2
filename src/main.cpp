#include "algo/Allocator.h"
#include "ui/Menu.h"
#include "io/OutputWriter.h"
#include "io/Parser.h"
#include "core/Web.h"

#include <cstdlib>
#include <iostream>
#include <string>

namespace {

// Batch mode: ./myProg -b ranges.txt registers.txt allocation.txt
// Parser already prepends basic/ranges/ and basic/registers/ when no '/' found.
int runBatch(const std::string &rangesFile,
             const std::string &configFile,
             const std::string &outputFile) {
    try {
        auto ranges = Parser::parseRanges(rangesFile);
        auto cfg    = Parser::parseConfig(configFile);
        auto webs   = buildWebs(ranges);

        AllocResult res;
        if      (cfg.algorithm == "basic")     res = Allocator::basic(webs, cfg.registers);
        else if (cfg.algorithm == "spilling")  res = Allocator::spilling(webs, cfg.registers, cfg.parameter);
        else if (cfg.algorithm == "splitting") res = Allocator::splitting(webs, cfg.registers, cfg.parameter);
        else if (cfg.algorithm == "free")      res = Allocator::freeStrategy(webs, cfg.registers);
        else {
            std::cerr << "Unknown algorithm: " << cfg.algorithm << '\n';
            return 2;
        }

        // Notes and warnings go to stderr so they don't pollute the output file
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
              << "  " << prog << "\n"
              << "        Interactive menu mode.\n"
              << "        Range files:  basic/ranges/<file>\n"
              << "        Config files: basic/registers/<file>\n\n"
              << "  " << prog << " -b <ranges.txt> <registers.txt> <output.txt>\n"
              << "        Batch mode.  Filenames only (no path) -> conventional dirs.\n"
              << "        Full relative paths also accepted.\n";
}

} // namespace

int main(int argc, char **argv) {
    // No arguments -> interactive menu
    if (argc == 1) return Menu::run();

    std::string a1 = argv[1];
    if (a1 == "-h" || a1 == "--help") { printUsage(argv[0]); return 0; }

    if (a1 == "-b") {
        if (argc != 5) { printUsage(argv[0]); return 2; }
        return runBatch(argv[2], argv[3], argv[4]);
    }

    printUsage(argv[0]);
    return 2;
}