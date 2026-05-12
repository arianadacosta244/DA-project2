/**
 * @file Menu.cpp
 * @brief Interactive CLI tying together parsing, web construction, graph
 *        building, allocation, and output emission.
 */
#include "Menu.h"

#include "Allocator.h"
#include "InterferenceBuilder.h"
#include "OutputWriter.h"
#include "Parser.h"
#include "Web.h"

#include <iostream>
#include <optional>
#include <string>

namespace {

struct State {
    std::optional<std::vector<LiveRange>> ranges;
    std::optional<AllocatorConfig>        config;
    std::optional<std::vector<Web>>       webs;
    std::optional<AllocResult>            result;
    std::string outputFile = "allocation.txt";
};

void banner() {
    std::cout << "\n=== Compiler Register Allocation Tool — DA 2026, Project 2 ===\n";
}

void printMenu() {
    std::cout <<
        "\n----------------------------------------------------------\n"
        " 1) Load live ranges file\n"
        " 2) Load configuration file (registers + algorithm)\n"
        " 3) Build webs (merge live ranges)\n"
        " 4) Show interference graph (edge list)\n"
        " 5) Run register allocation\n"
        " 6) Show last result on screen\n"
        " 7) Write last result to output file\n"
        " 8) Set output filename (current: ";
}

std::string ask(const std::string &prompt) {
    std::cout << prompt;
    std::string s;
    std::getline(std::cin, s);
    return s;
}

void doLoadRanges(State &st) {
    std::string f = ask("  ranges filename: ");
    try {
        st.ranges = Parser::parseRanges(f);
        std::cout << "  loaded " << st.ranges->size() << " live range(s).\n";
        st.webs.reset();
        st.result.reset();
    } catch (const std::exception &e) {
        std::cerr << "  error: " << e.what() << '\n';
    }
}

void doLoadConfig(State &st) {
    std::string f = ask("  config filename: ");
    try {
        st.config = Parser::parseConfig(f);
        std::cout << "  registers=" << st.config->registers
                  << " algorithm=" << st.config->algorithm;
        if (st.config->algorithm == "spilling" || st.config->algorithm == "splitting")
            std::cout << " parameter=" << st.config->parameter;
        std::cout << '\n';
    } catch (const std::exception &e) {
        std::cerr << "  error: " << e.what() << '\n';
    }
}

void doBuildWebs(State &st) {
    if (!st.ranges) { std::cout << "  load a ranges file first.\n"; return; }
    st.webs = buildWebs(*st.ranges);
    std::cout << "  built " << st.webs->size() << " web(s):\n";
    for (const auto &w : *st.webs)
        std::cout << "    web" << w.id << " (" << w.variable << "): " << w.serialize() << '\n';
    st.result.reset();
}

void doShowGraph(State &st) {
    if (!st.webs) { std::cout << "  build webs first.\n"; return; }
    auto g = InterferenceBuilder::build(*st.webs);
    std::cout << "  vertices: " << g->getNumVertex() << '\n';
    std::cout << "  edges (interferences):\n";
    bool any = false;
    for (auto *v : g->getVertexSet()) {
        for (auto &e : v->getAdj()) {
            if (v->getInfo() < e.getDest()->getInfo()) {
                std::cout << "    web" << v->getInfo() << " -- web" << e.getDest()->getInfo() << '\n';
                any = true;
            }
        }
    }
    if (!any) std::cout << "    (none)\n";
}

void doAllocate(State &st) {
    if (!st.webs)   { std::cout << "  build webs first.\n"; return; }
    if (!st.config) { std::cout << "  load configuration first.\n"; return; }

    const auto &alg = st.config->algorithm;
    if      (alg == "basic")     st.result = Allocator::basic(*st.webs, st.config->registers);
    else if (alg == "spilling")  st.result = Allocator::spilling(*st.webs, st.config->registers, st.config->parameter);
    else if (alg == "splitting") st.result = Allocator::splitting(*st.webs, st.config->registers, st.config->parameter);
    else if (alg == "free")      st.result = Allocator::freeStrategy(*st.webs, st.config->registers);
    else { std::cerr << "  internal error: unknown algorithm '" << alg << "'\n"; return; }

    std::cout << "  " << st.result->note << '\n';
    if (!st.result->feasible)
        std::cerr << "  WARNING: register assignment infeasible with provided settings — all webs spilled to memory.\n";
}

void doShowResult(State &st) {
    if (!st.result) { std::cout << "  run allocation first.\n"; return; }
    OutputWriter::write(std::cout, *st.result);
}

void doWriteResult(State &st) {
    if (!st.result) { std::cout << "  run allocation first.\n"; return; }
    try {
        OutputWriter::writeToFile(st.outputFile, *st.result);
        std::cout << "  wrote " << st.outputFile << '\n';
    } catch (const std::exception &e) {
        std::cerr << "  error: " << e.what() << '\n';
    }
}

void doSetOutput(State &st) {
    std::string f = ask("  output filename: ");
    if (!f.empty()) st.outputFile = f;
}

} // namespace

int Menu::run() {
    banner();
    State st;
    while (true) {
        printMenu();
        std::cout << st.outputFile << ")\n"
                     " 9) Run end-to-end (1..2..3..5..7)\n"
                     " 0) Exit\n"
                     "----------------------------------------------------------\n"
                     " > ";
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
        else if (choice == "9") {
            doLoadRanges(st);
            if (st.ranges) doLoadConfig(st);
            if (st.ranges && st.config) doBuildWebs(st);
            if (st.webs && st.config)   doAllocate(st);
            if (st.result)              doWriteResult(st);
        }
        else if (choice == "0" || choice == "q") break;
        else std::cout << "  unknown choice\n";
    }
    return 0;
}
