#include "Parser.h"

#include <algorithm>
#include <cctype>
#include <fstream>
#include <sstream>
#include <stdexcept>

namespace {

std::ifstream openWithFallback(const std::string& filename, const std::string& fallbackDir) {
    std::ifstream in(filename);
    
    // If opening directly fails, try the fallback directory
    if (!in.is_open()) {
        std::string fallbackPath = fallbackDir + "/" + filename;
        in.open(fallbackPath);
    }
    
    return in;
}

void trim(std::string &s) {
    auto notSpace = [](unsigned char c) { return !std::isspace(c); };
    s.erase(s.begin(), std::find_if(s.begin(), s.end(), notSpace));
    s.erase(std::find_if(s.rbegin(), s.rend(), notSpace).base(), s.end());
}

bool isSkippable(const std::string &line) {
    for (char c : line) {
        if (std::isspace(static_cast<unsigned char>(c))) continue;
        return c == '#';
    }
    return true;
}

ProgramPoint parsePoint(std::string tok, const std::string &context) {
    trim(tok);
    if (tok.empty()) throw std::runtime_error("Empty program point in: " + context);
    char marker = ' ';
    if (tok.back() == '+' || tok.back() == '-') {
        marker = tok.back();
        tok.pop_back();
        trim(tok);
    }
    try {
        std::size_t pos = 0;
        int line = std::stoi(tok, &pos);
        if (pos != tok.size())
            throw std::runtime_error("Garbage after line number in: " + context);
        if (line <= 0) throw std::runtime_error("Non-positive line in: " + context);
        return {line, marker};
    } catch (const std::invalid_argument &) {
        throw std::runtime_error("Bad line number in: " + context);
    }
}

}

std::vector<LiveRange> Parser::parseRanges(const std::string &filename) {
    std::ifstream in = openWithFallback(filename, "basic/ranges");
    if (!in) throw std::runtime_error("Cannot open ranges file: " + filename);

    std::vector<LiveRange> out;
    std::string line;
    int lineNo = 0;
    while (std::getline(in, line)) {
        ++lineNo;
        if (isSkippable(line)) continue;

        auto colon = line.find(':');
        if (colon == std::string::npos)
            throw std::runtime_error("Missing ':' in ranges file at line " + std::to_string(lineNo));

        std::string var = line.substr(0, colon);
        trim(var);
        if (var.empty())
            throw std::runtime_error("Empty variable name at line " + std::to_string(lineNo));

        LiveRange range;
        range.variable = var;

        std::string rest = line.substr(colon + 1);
        std::stringstream ss(rest);
        std::string tok;
        while (std::getline(ss, tok, ',')) {
            const std::string ctx = "line " + std::to_string(lineNo) + " (\"" + tok + "\")";
            range.points.push_back(parsePoint(tok, ctx));
        }
        if (range.points.empty())
            throw std::runtime_error("Range with no points at line " + std::to_string(lineNo));

        out.push_back(std::move(range));
    }
    return out;
}

AllocatorConfig Parser::parseConfig(const std::string &filename) {
    std::ifstream in = openWithFallback(filename, "basic/registers");
    if (!in) throw std::runtime_error("Cannot open config file: " + filename);

    AllocatorConfig cfg;
    bool sawRegisters = false;
    bool sawAlgorithm = false;

    std::string line;
    int lineNo = 0;
    while (std::getline(in, line)) {
        ++lineNo;
        if (isSkippable(line)) continue;

        auto colon = line.find(':');
        if (colon == std::string::npos)
            throw std::runtime_error("Missing ':' in config at line " + std::to_string(lineNo));
        std::string key = line.substr(0, colon);
        std::string val = line.substr(colon + 1);
        trim(key);
        trim(val);
        std::transform(key.begin(), key.end(), key.begin(),
                       [](unsigned char c) { return std::tolower(c); });

        if (key == "registers") {
            try {
                cfg.registers = std::stoi(val);
            } catch (...) {
                throw std::runtime_error("Bad 'registers' value at line " + std::to_string(lineNo));
            }
            if (cfg.registers <= 0)
                throw std::runtime_error("'registers' must be positive (line " + std::to_string(lineNo) + ")");
            sawRegisters = true;
        } else if (key == "algorithm") {
            auto comma = val.find(',');
            std::string name = (comma == std::string::npos) ? val : val.substr(0, comma);
            std::string param = (comma == std::string::npos) ? "" : val.substr(comma + 1);
            trim(name);
            trim(param);
            std::transform(name.begin(), name.end(), name.begin(),
                           [](unsigned char c) { return std::tolower(c); });
            if (name != "basic" && name != "spilling" && name != "splitting" && name != "free")
                throw std::runtime_error("Unknown algorithm '" + name + "'");
            cfg.algorithm = name;
            if (name == "spilling" || name == "splitting") {
                if (param.empty())
                    throw std::runtime_error("'" + name + "' requires a numeric parameter");
                try { cfg.parameter = std::stoi(param); }
                catch (...) { throw std::runtime_error("Bad parameter for '" + name + "'"); }
                if (cfg.parameter < 0)
                    throw std::runtime_error("Parameter for '" + name + "' must be >= 0");
            }
            sawAlgorithm = true;
        }
    }
    if (!sawRegisters) throw std::runtime_error("Config missing 'registers' directive");
    if (!sawAlgorithm) throw std::runtime_error("Config missing 'algorithm' directive");
    return cfg;
}
