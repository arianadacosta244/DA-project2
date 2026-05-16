#include "Menu.h"

#include "../algo/Allocator.h"
#include "../algo/InterferenceBuilder.h"
#include "../io/OutputWriter.h"
#include "../io/Parser.h"
#include "../core/Web.h"

#include <iostream>
#include <optional>
#include <string>

namespace {

struct State {
    std::optional<std::vector<LiveRange>> ranges;
    std::optional<AllocatorConfig>        config;
    std::optional<std::vector<Web>>       webs;
    std::optional<AllocResult>            result;
    std::string rangesFile  = "";
    std::string configFile  = "";
    std::string outputFile  = "allocation.txt";
};

void banner() {
    std::cout << "\n============================================\n";
    std::cout << "   Compiler Register Allocation Tool\n";
    std::cout << "============================================\n";
    std::cout << "  Ranges files  : basic/ranges/\n";
    std::cout << "  Register files: basic/registers/\n";
    std::cout << "============================================\n";
}

void printMenu(const State &st) {
    std::cout << "\n----------------------------------------------------------\n";
    std::cout << " 1) Load live ranges file"
              << (st.ranges ? "  [loaded: " + st.rangesFile + "]" : "  [none]") << "\n";
    std::cout << " 2) Load configuration file (registers + algorithm)"
              << (st.config ? "  [loaded: " + st.configFile + "]" : "  [none]") << "\n";
    std::cout << " 3) Build webs (merge live ranges)"
              << (st.webs ? "  [" + std::to_string(st.webs->size()) + " web(s)]" : "  [not built]") << "\n";
    std::cout << " 4) Show interference graph (edge list)\n";
    std::cout << " 5) Run register allocation"
              << (st.result ? "  [done: " + st.result->note + "]" : "  [not run]") << "\n";
    std::cout << " 6) Show last result on screen\n";
    std::cout << " 7) Write last result to output file  [" << st.outputFile << "]\n";
    std::cout << " 8) Set output filename\n";
    std::cout << " 9) Run end-to-end (load -> build -> allocate -> write)\n";
    std::cout << " 0) Exit\n";
    std::cout << "----------------------------------------------------------\n > ";
}

std::string ask(const std::string &prompt) {
    std::cout << prompt;
    std::string s;
    std::getline(std::cin, s);
    return s;
}

// ---- individual actions ---------------------------------------------------

void doLoadRanges(State &st) {
    std::string f = ask("  ranges filename (inside basic/ranges/): ");
    if (f.empty()) { std::cout << "  cancelled.\n"; return; }
    try {
        st.ranges = Parser::parseRanges(f);
        st.rangesFile = f;
        std::cout << "  OK - loaded " << st.ranges->size() << " live range(s) from basic/ranges/" << f << "\n";
        st.webs.reset();
        st.result.reset();
    } catch (const std::exception &e) {
        std::cerr << "  ERROR: " << e.what() << '\n';
    }
}

void doLoadConfig(State &st) {
    std::string f = ask("  config filename (inside basic/registers/): ");
    if (f.empty()) { std::cout << "  cancelled.\n"; return; }
    try {
        st.config = Parser::parseConfig(f);
        st.configFile = f;
        std::cout << "  OK - registers=" << st.config->registers
                  << "  algorithm=" << st.config->algorithm;
        if (st.config->algorithm == "spilling" || st.config->algorithm == "splitting")
            std::cout << "  parameter=" << st.config->parameter;
        std::cout << '\n';
        st.result.reset();
    } catch (const std::exception &e) {
        std::cerr << "  ERROR: " << e.what() << '\n';
    }
}

void doBuildWebs(State &st) {
    if (!st.ranges) { std::cout << "  Load a ranges file first (option 1).\n"; return; }
    st.webs = buildWebs(*st.ranges);
    std::cout << "  OK - built " << st.webs->size() << " web(s):\n";
    for (const auto &w : *st.webs)
        std::cout << "    web" << w.id << " [" << w.variable << "]: " << w.serialize() << '\n';
    st.result.reset();
}

void doShowGraph(State &st) {
    if (!st.webs) { std::cout << "  Build webs first (option 3).\n"; return; }
    auto g = InterferenceBuilder::build(*st.webs);
    std::cout << "  Interference graph: " << g->getNumVertex() << " node(s)\n";
    bool any = false;
    for (auto *v : g->getVertexSet()) {
        for (auto &e : v->getAdj()) {
            if (v->getInfo() < e.getDest()->getInfo()) {
                std::cout << "    web" << v->getInfo()
                          << " -- web" << e.getDest()->getInfo() << '\n';
                any = true;
            }
        }
    }
    if (!any) std::cout << "    (no edges - no interferences)\n";
}

void doAllocate(State &st) {
    if (!st.webs)   { std::cout << "  Build webs first (option 3).\n"; return; }
    if (!st.config) { std::cout << "  Load configuration first (option 2).\n"; return; }

    const auto &alg = st.config->algorithm;
    if      (alg == "basic")     st.result = Allocator::basic(*st.webs, st.config->registers);
    else if (alg == "spilling")  st.result = Allocator::spilling(*st.webs, st.config->registers, st.config->parameter);
    else if (alg == "splitting") st.result = Allocator::splitting(*st.webs, st.config->registers, st.config->parameter);
    else if (alg == "free")      st.result = Allocator::freeStrategy(*st.webs, st.config->registers);
    else { std::cerr << "  internal error: unknown algorithm '" << alg << "'\n"; return; }

    std::cout << "  " << st.result->note << '\n';
    if (!st.result->feasible)
        std::cerr << "  WARNING: register assignment infeasible - all webs spilled to memory.\n";
}

void doShowResult(State &st) {
    if (!st.result) { std::cout << "  Run allocation first (option 5).\n"; return; }
    OutputWriter::write(std::cout, *st.result);
}

void doWriteResult(State &st) {
    if (!st.result) { std::cout << "  Run allocation first (option 5).\n"; return; }
    try {
        OutputWriter::writeToFile(st.outputFile, *st.result);
        std::cout << "  Written to: " << st.outputFile << '\n';
    } catch (const std::exception &e) {
        std::cerr << "  ERROR: " << e.what() << '\n';
    }
}

void doSetOutput(State &st) {
    std::string f = ask("  new output filename: ");
    if (!f.empty()) {
        st.outputFile = f;
        std::cout << "  output file set to: " << st.outputFile << '\n';
    }
}

void doEndToEnd(State &st) {
    std::cout << "\n  --- End-to-end run ---\n";
    doLoadRanges(st);
    if (!st.ranges) return;
    doLoadConfig(st);
    if (!st.config) return;
    doBuildWebs(st);
    if (!st.webs) return;
    doAllocate(st);
    if (!st.result) return;
    doWriteResult(st);
    std::cout << "  --- Done ---\n";
}

} // anonymous namespace

int Menu::run() {
    banner();
    State st;
    while (true) {
        printMenu(st);
        std::string choice;
        if (!std::getline(std::cin, choice)) break;
        if      (choice == "1") doLoadRanges(st);
        else if (choice == "2") doLoadConfig(st);
        else if (choice == "3") doBuildWebs(st);
        else if (choice == "4") doShowGraph(st);
        else if (choice == "5") doAllocate(st);
        else if (choice == "6") doShowResult(st);
        else if (choice == "7") doWriteResult(st);
        else if (choice == "8") doSetOutput(st);
        else if (choice == "9") doEndToEnd(st);
        else if (choice == "0" || choice == "q") { std::cout << "  Goodbye.\n"; break; }
        else std::cout << "  Unknown option. Please enter 0-9.\n";
    }
    return 0;
}