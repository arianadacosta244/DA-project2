#include "InterferenceBuilder.h"

std::unique_ptr<Graph<int>> InterferenceBuilder::build(const std::vector<Web> &webs) {
    auto g = std::make_unique<Graph<int>>();
    for (const auto &w : webs) g->addVertex(w.id);
    for (std::size_t i = 0; i < webs.size(); ++i) {
        for (std::size_t j = i + 1; j < webs.size(); ++j) {
            if (webs[i].interferesWith(webs[j]))
                g->addEdge(webs[i].id, webs[j].id);
        }
    }
    return g;
}
