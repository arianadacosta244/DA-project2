#pragma once

#include <algorithm>
#include <cstddef>
#include <vector>

template <class T> class Edge;
template <class T> class Graph;

template <class T>
class Vertex {
    T info;
    std::vector<Edge<T>> adj;
    bool visited = false;
    bool active = true;
    int color = -1;

    friend class Graph<T>;

public:
    explicit Vertex(T in) : info(in) {}

    T getInfo() const { return info; }
    std::vector<Edge<T>> &getAdj() { return adj; }
    const std::vector<Edge<T>> &getAdj() const { return adj; }

    bool isVisited() const { return visited; }
    void setVisited(bool v) { visited = v; }

    bool isActive() const { return active; }
    void setActive(bool a) { active = a; }

    int getColor() const { return color; }
    void setColor(int c) { color = c; }

    int activeDegree() const {
        int d = 0;
        for (const auto &e : adj)
            if (e.getDest()->isActive()) ++d;
        return d;
    }
};

template <class T>
class Edge {
    Vertex<T> *dest;
    double weight;

public:
    Edge(Vertex<T> *d, double w) : dest(d), weight(w) {}
    Vertex<T> *getDest() const { return dest; }
    double getWeight() const { return weight; }
};

template <class T>
class Graph {
    std::vector<Vertex<T>*> vertexSet;

public:
    Graph() = default;

    ~Graph() {
        for (auto *v : vertexSet) delete v;
    }

    Graph(const Graph &) = delete;
    Graph &operator=(const Graph &) = delete;

    Vertex<T> *findVertex(const T &in) const {
        for (auto *v : vertexSet)
            if (v->getInfo() == in) return v;
        return nullptr;
    }

    bool addVertex(const T &in) {
        if (findVertex(in) != nullptr) return false;
        vertexSet.push_back(new Vertex<T>(in));
        return true;
    }

    bool addEdge(const T &src, const T &dst, double w = 1.0) {
        auto *s = findVertex(src);
        auto *d = findVertex(dst);
        if (!s || !d || s == d) return false;
        for (const auto &e : s->getAdj())
            if (e.getDest() == d) return false;
        s->getAdj().emplace_back(d, w);
        d->getAdj().emplace_back(s, w);
        return true;
    }

    int getNumVertex() const { return static_cast<int>(vertexSet.size()); }

    const std::vector<Vertex<T>*> &getVertexSet() const { return vertexSet; }
    std::vector<Vertex<T>*> &getVertexSet() { return vertexSet; }

    void resetState() {
        for (auto *v : vertexSet) {
            v->active = true;
            v->color = -1;
            v->visited = false;
        }
    }
};
