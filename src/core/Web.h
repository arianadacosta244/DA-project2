#pragma once

#include <string>
#include <vector>

struct ProgramPoint {
    int line;
    char marker;
};

struct LiveRange {
    std::string variable;
    std::vector<ProgramPoint> points;
};

class Web {
public:
    int id = -1;
    std::string variable;
    std::vector<ProgramPoint> points;

    static Web merge(const Web &a, const Web &b);
    static bool overlap(const Web &a, const Web &b);

    bool containsLine(int line) const;
    char markerAt(int line) const;
    bool interferesWith(const Web &other) const;
    std::string serialize() const;
};

std::vector<Web> buildWebs(const std::vector<LiveRange> &ranges);
