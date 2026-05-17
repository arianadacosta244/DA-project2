#include "OutputWriter.h"

#include <fstream>
#include <stdexcept>

void OutputWriter::write(std::ostream &os, const AllocResult &result) {
    os << "webs: " << result.webs.size() << '\n';
    for (const auto &w : result.webs)
        os << "web" << w.id << ": " << w.serialize() << '\n';

    if (!result.feasible) {
        os << "registers: 0\n";
        for (const auto &w : result.webs)
            os << "M: web" << w.id << '\n';
        return;
    }

    os << "registers: " << result.registersUsed << '\n';
    for (int r = 0; r < result.registersUsed; ++r) {
        for (std::size_t i = 0; i < result.webs.size(); ++i) {
            if (!result.spilled[i] && result.reg[i] == r)
                os << "r" << r << ": web" << result.webs[i].id << '\n';
        }
    }
    for (std::size_t i = 0; i < result.webs.size(); ++i) {
        if (result.spilled[i])
            os << "M: web" << result.webs[i].id << '\n';
    }
}

void OutputWriter::writeToFile(const std::string &filename, const AllocResult &result) {
    std::string targetPath = filename;
    if (targetPath.find('/') == std::string::npos && targetPath.find('\\') == std::string::npos) {
        targetPath = "basic/output/" + targetPath;
    }

    std::ofstream out(targetPath);
    if (!out) throw std::runtime_error("Cannot open output file: " + filename);
    write(out, result);
}
