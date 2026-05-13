#include "Allocator.h"
#include "InterferenceBuilder.h"

#include <algorithm>
#include <cassert>
#include <memory>
#include <set>
#include <sstream>

namespace {

struct ColorAttempt {
    bool ok = false;
    std::vector<int>  color;
    std::vector<bool> spilled;
};

ColorAttempt chaitinPass(Graph<int> &g, int K, int spillBudget) {
    ColorAttempt res;
    int n = g.getNumVertex();
    res.color.assign(n, -1);
    res.spilled.assign(n, false);
    if (K <= 0) return res;

    g.resetState();
    std::vector<Vertex<int>*> stack;
    int spillsUsed = 0;
    int active = n;

    auto pickLowDegree = [&]() -> Vertex<int>* {
        for (auto *v : g.getVertexSet())
            if (v->isActive() && v->activeDegree() < K) return v;
        return nullptr;
    };
    auto pickHighDegree = [&]() -> Vertex<int>* {
        Vertex<int> *best = nullptr;
        int bestDeg = -1;
        for (auto *v : g.getVertexSet()) {
            if (!v->isActive()) continue;
            int d = v->activeDegree();
            if (d > bestDeg) { best = v; bestDeg = d; }
        }
        return best;
    };

    while (active > 0) {
        if (Vertex<int> *v = pickLowDegree()) {
            v->setActive(false);
            stack.push_back(v);
            --active;
            continue;
        }
        if (spillsUsed >= spillBudget) return res;
        Vertex<int> *vic = pickHighDegree();
        if (!vic) return res;
        vic->setActive(false);
        res.spilled[vic->getInfo()] = true;
        ++spillsUsed;
        --active;
    }

    while (!stack.empty()) {
        Vertex<int> *v = stack.back();
        stack.pop_back();
        v->setActive(true);
        std::vector<bool> used(K, false);
        for (auto &e : v->getAdj()) {
            auto *nbr = e.getDest();
            if (nbr->isActive() && nbr->getColor() >= 0 && nbr->getColor() < K)
                used[nbr->getColor()] = true;
        }
        int chosen = -1;
        for (int c = 0; c < K; ++c) if (!used[c]) { chosen = c; break; }
        if (chosen < 0) return res;
        v->setColor(chosen);
    }

    for (auto *v : g.getVertexSet()) res.color[v->getInfo()] = v->getColor();
    res.ok = true;
    return res;
}

int countColors(const std::vector<int> &color) {
    std::set<int> s;
    for (int c : color) if (c >= 0) s.insert(c);
    return static_cast<int>(s.size());
}

void compactColors(std::vector<int> &color) {
    std::vector<int> remap;
    for (int c : color) if (c >= 0 && std::find(remap.begin(), remap.end(), c) == remap.end())
        remap.push_back(c);
    std::sort(remap.begin(), remap.end());
    for (int &c : color) {
        if (c < 0) continue;
        auto it = std::find(remap.begin(), remap.end(), c);
        c = static_cast<int>(it - remap.begin());
    }
}

AllocResult finalize(std::vector<Web> webs,
                     std::vector<int>  color,
                     std::vector<bool> spilled,
                     std::string note) {
    compactColors(color);
    AllocResult r;
    r.feasible      = true;
    r.webs          = std::move(webs);
    r.reg           = std::move(color);
    r.spilled       = std::move(spilled);
    r.registersUsed = countColors(r.reg);
    r.note          = std::move(note);
    return r;
}

AllocResult infeasible(std::vector<Web> webs, std::string note) {
    AllocResult r;
    r.feasible      = false;
    r.registersUsed = 0;
    r.webs          = std::move(webs);
    r.reg.assign(r.webs.size(), -1);
    r.spilled.assign(r.webs.size(), true);
    r.note          = std::move(note);
    return r;
}

std::vector<Web> splitWeb(const Web &w) {
    std::vector<int> defs;
    for (std::size_t i = 0; i < w.points.size(); ++i)
        if (w.points[i].marker == '+') defs.push_back(static_cast<int>(i));

    if (defs.size() < 2) {
        if (w.points.size() < 2) return {w};
        int mid = static_cast<int>(w.points.size()) / 2;
        Web a, b;
        a.variable = b.variable = w.variable;
        for (int i = 0; i < mid; ++i) a.points.push_back(w.points[i]);
        for (int i = mid; i < (int)w.points.size(); ++i) b.points.push_back(w.points[i]);
        if (!a.points.empty()) a.points.back().marker  = '-';
        if (!b.points.empty()) b.points.front().marker = '+';
        return {a, b};
    }

    std::vector<Web> subs;
    for (std::size_t k = 0; k < defs.size(); ++k) {
        int start = defs[k];
        int end   = (k + 1 < defs.size()) ? defs[k + 1] : (int)w.points.size();
        Web sub;
        sub.variable = w.variable;
        for (int i = start; i < end; ++i) sub.points.push_back(w.points[i]);
        if (!sub.points.empty() && k + 1 < defs.size() && sub.points.back().marker != '-')
            sub.points.back().marker = '-';
        subs.push_back(std::move(sub));
    }
    return subs;
}

void renumber(std::vector<Web> &webs) {
    for (int i = 0; i < (int)webs.size(); ++i) webs[i].id = i;
}

}

AllocResult Allocator::basic(const std::vector<Web> &webs, int maxRegs) {
    auto g = InterferenceBuilder::build(webs);
    for (int K = 1; K <= maxRegs; ++K) {
        auto a = chaitinPass(*g, K, 0);
        if (a.ok) {
            std::ostringstream note;
            note << "basic: colored with " << K << " register(s)";
            return finalize(webs, std::move(a.color), std::move(a.spilled), note.str());
        }
    }
    return infeasible(webs, "basic: cannot color with " + std::to_string(maxRegs) + " register(s)");
}

AllocResult Allocator::spilling(const std::vector<Web> &webs, int maxRegs, int spillBudget) {
    auto g = InterferenceBuilder::build(webs);
    for (int spills = 0; spills <= spillBudget; ++spills) {
        for (int K = 1; K <= maxRegs; ++K) {
            auto a = chaitinPass(*g, K, spills);
            if (a.ok) {
                std::ostringstream note;
                note << "spilling: K=" << K << " register(s), "
                     << std::count(a.spilled.begin(), a.spilled.end(), true)
                     << " web(s) spilled";
                return finalize(webs, std::move(a.color), std::move(a.spilled), note.str());
            }
        }
    }
    return infeasible(webs,
        "spilling: budget of " + std::to_string(spillBudget) + " not sufficient");
}

AllocResult Allocator::splitting(const std::vector<Web> &webs, int maxRegs, int splitBudget) {
    std::vector<Web> working = webs;

    auto tryColor = [&](const std::vector<Web> &ws) -> ColorAttempt {
        auto g = InterferenceBuilder::build(ws);
        for (int K = 1; K <= maxRegs; ++K) {
            auto a = chaitinPass(*g, K, 0);
            if (a.ok) return a;
        }
        return {};
    };

    auto a0 = tryColor(working);
    if (a0.ok)
        return finalize(working, std::move(a0.color), std::move(a0.spilled),
                        "splitting: 0 splits needed (basic succeeded)");

    int splitsDone = 0;
    while (splitsDone < splitBudget) {
        auto g = InterferenceBuilder::build(working);
        int target = -1;
        int bestDeg = -1;
        for (auto *v : g->getVertexSet()) {
            int d = static_cast<int>(v->getAdj().size());
            if (d > bestDeg) { bestDeg = d; target = v->getInfo(); }
        }
        if (target < 0) break;

        auto subs = splitWeb(working[target]);
        if (subs.size() < 2) break;

        working[target] = subs[0];
        for (std::size_t k = 1; k < subs.size(); ++k) working.push_back(subs[k]);
        renumber(working);
        ++splitsDone;

        auto a = tryColor(working);
        if (a.ok) {
            std::ostringstream note;
            note << "splitting: succeeded after " << splitsDone << " split(s)";
            return finalize(working, std::move(a.color), std::move(a.spilled), note.str());
        }
    }
    return infeasible(working,
        "splitting: budget of " + std::to_string(splitBudget) + " not sufficient");
}

AllocResult Allocator::freeStrategy(const std::vector<Web> &webs, int maxRegs) {
    auto g = InterferenceBuilder::build(webs);
    int n = g->getNumVertex();

    for (int K = 1; K <= maxRegs; ++K) {
        g->resetState();
        bool ok = true;
        for (int step = 0; step < n; ++step) {
            Vertex<int> *best = nullptr;
            int bestSat = -1, bestDeg = -1;
            for (auto *v : g->getVertexSet()) {
                if (v->getColor() >= 0) continue;
                std::set<int> nbColors;
                for (auto &e : v->getAdj()) {
                    int c = e.getDest()->getColor();
                    if (c >= 0) nbColors.insert(c);
                }
                int sat = static_cast<int>(nbColors.size());
                int deg = static_cast<int>(v->getAdj().size());
                if (sat > bestSat || (sat == bestSat && deg > bestDeg)) {
                    best = v; bestSat = sat; bestDeg = deg;
                }
            }
            if (!best) { ok = false; break; }
            std::vector<bool> used(K, false);
            for (auto &e : best->getAdj()) {
                int c = e.getDest()->getColor();
                if (c >= 0 && c < K) used[c] = true;
            }
            int chosen = -1;
            for (int c = 0; c < K; ++c) if (!used[c]) { chosen = c; break; }
            if (chosen < 0) { ok = false; break; }
            best->setColor(chosen);
        }
        if (ok) {
            std::vector<int>  color(n, -1);
            std::vector<bool> spilled(n, false);
            for (auto *v : g->getVertexSet()) color[v->getInfo()] = v->getColor();
            std::ostringstream note;
            note << "free (DSatur): colored with " << K << " register(s)";
            return finalize(webs, std::move(color), std::move(spilled), note.str());
        }
    }
    return infeasible(webs, "free (DSatur): cannot color with " + std::to_string(maxRegs) + " register(s)");
}
