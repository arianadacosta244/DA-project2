#include "Web.h"

#include <algorithm>
#include <map>
#include <numeric>
#include <set>
#include <sstream>
#include <unordered_map>

bool Web::containsLine(int line) const {
    auto it = std::lower_bound(points.begin(), points.end(), line,
        [](const ProgramPoint &p, int v) { return p.line < v; });
    return it != points.end() && it->line == line;
}

char Web::markerAt(int line) const {
    auto it = std::lower_bound(points.begin(), points.end(), line,
        [](const ProgramPoint &p, int v) { return p.line < v; });
    if (it == points.end() || it->line != line) return '?';
    return it->marker;
}

bool Web::overlap(const Web &a, const Web &b) {
    std::size_t i = 0, j = 0;
    while (i < a.points.size() && j < b.points.size()) {
        if (a.points[i].line == b.points[j].line) return true;
        if (a.points[i].line < b.points[j].line) ++i; else ++j;
    }
    return false;
}

bool Web::interferesWith(const Web &other) const {
    std::size_t i = 0, j = 0;
    while (i < points.size() && j < other.points.size()) {
        const auto &p = points[i];
        const auto &q = other.points[j];
        if (p.line == q.line) {
            bool plusMinus = (p.marker == '+' && q.marker == '-') ||
                             (p.marker == '-' && q.marker == '+');
            if (!plusMinus) return true;
            ++i; ++j;
        } else if (p.line < q.line) {
            ++i;
        } else {
            ++j;
        }
    }
    return false;
}

Web Web::merge(const Web &a, const Web &b) {
    Web out;
    out.variable = a.variable;

    std::map<int, std::set<char>> markers;
    auto absorb = [&](const Web &w) {
        for (const auto &p : w.points) markers[p.line].insert(p.marker);
    };
    absorb(a);
    absorb(b);

    for (auto &[line, ms] : markers) {
        bool hasPlus  = ms.count('+') > 0;
        bool hasMinus = ms.count('-') > 0;
        char m = ' ';
        if (hasPlus && hasMinus)      m = ' ';
        else if (hasPlus)             m = '+';
        else if (hasMinus)            m = '-';
        else                          m = ' ';
        out.points.push_back({line, m});
    }
    return out;
}

std::string Web::serialize() const {
    std::ostringstream os;
    bool first = true;
    for (const auto &p : points) {
        if (!first) os << ',';
        first = false;
        os << p.line;
        if (p.marker == '+' || p.marker == '-') os << p.marker;
    }
    return os.str();
}

namespace {

Web rangeToWeb(const LiveRange &r) {
    Web w;
    w.variable = r.variable;
    w.points = r.points;
    std::sort(w.points.begin(), w.points.end(),
              [](const ProgramPoint &a, const ProgramPoint &b) { return a.line < b.line; });
    return w;
}

struct DSU {
    std::vector<int> parent, rank;
    explicit DSU(int n) : parent(n), rank(n, 0) {
        std::iota(parent.begin(), parent.end(), 0);
    }
    int find(int x) {
        while (parent[x] != x) { parent[x] = parent[parent[x]]; x = parent[x]; }
        return x;
    }
    void unite(int a, int b) {
        a = find(a); b = find(b);
        if (a == b) return;
        if (rank[a] < rank[b]) std::swap(a, b);
        parent[b] = a;
        if (rank[a] == rank[b]) ++rank[a];
    }
};

}

std::vector<Web> buildWebs(const std::vector<LiveRange> &ranges) {
    std::vector<Web> cand;
    cand.reserve(ranges.size());
    for (const auto &r : ranges) cand.push_back(rangeToWeb(r));

    std::unordered_map<std::string, std::vector<int>> byVar;
    for (int i = 0; i < (int)cand.size(); ++i) byVar[cand[i].variable].push_back(i);

    DSU dsu((int)cand.size());
    for (auto &[_, idxs] : byVar) {
        for (std::size_t a = 0; a < idxs.size(); ++a) {
            for (std::size_t b = a + 1; b < idxs.size(); ++b) {
                if (Web::overlap(cand[idxs[a]], cand[idxs[b]]))
                    dsu.unite(idxs[a], idxs[b]);
            }
        }
    }

    std::unordered_map<int, std::vector<int>> comps;
    for (int i = 0; i < (int)cand.size(); ++i) comps[dsu.find(i)].push_back(i);

    std::vector<Web> result;
    result.reserve(comps.size());
    for (auto &[_, idxs] : comps) {
        std::map<int, std::set<char>> markers;
        std::string var;
        for (int idx : idxs) {
            var = cand[idx].variable;
            for (const auto &p : cand[idx].points) markers[p.line].insert(p.marker);
        }
        Web w;
        w.variable = var;
        for (auto &[line, ms] : markers) {
            bool hasPlus  = ms.count('+') > 0;
            bool hasMinus = ms.count('-') > 0;
            char m = ' ';
            if (hasPlus && hasMinus)      m = ' ';
            else if (hasPlus)             m = '+';
            else if (hasMinus)            m = '-';
            w.points.push_back({line, m});
        }
        result.push_back(std::move(w));
    }

    std::sort(result.begin(), result.end(), [](const Web &a, const Web &b) {
        if (a.variable != b.variable) return a.variable < b.variable;
        int la = a.points.empty() ? 0 : a.points.front().line;
        int lb = b.points.empty() ? 0 : b.points.front().line;
        return la < lb;
    });

    for (int i = 0; i < (int)result.size(); ++i) result[i].id = i;
    return result;
}
