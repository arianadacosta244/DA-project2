/**
 * @file Graph.h
 * @brief Generic templated graph class (FEUP DA style) with minor additions
 *        to support register-allocation graph coloring.
 *
 * Conceptual additions over the lecture template:
 *   - per-vertex @c active flag, so we can simulate node removal during
 *     Chaitin's simplification phase without rebuilding the adjacency list
 *     (effective degree is computed over active neighbours only);
 *   - per-vertex @c color slot used by the coloring algorithms.
 *
 * Both additions are purely auxiliary: the underlying topology (vertex set
 * and adjacency lists) is the standard one taught in the TP lectures.
 */
#pragma once

#include <algorithm>
#include <cstddef>
#include <vector>

template <class T> class Edge;
template <class T> class Graph;

/**
 * @brief A vertex of @ref Graph.
 *
 * Stores generic information of type @c T plus auxiliary state used by
 * register-allocation algorithms (@c active, @c color).
 */
template <class T>
class Vertex {
    T info;                       ///< user payload (e.g. web id)
    std::vector<Edge<T>> adj;     ///< outgoing edges
    bool visited = false;         ///< generic traversal flag
    bool active = true;           ///< false ⇒ logically removed
    int color = -1;               ///< assigned color (or -1 = uncolored)

    friend class Graph<T>;

public:
    /// @param in vertex payload
    explicit Vertex(T in) : info(in) {}

    /// @return the stored payload.
    T getInfo() const { return info; }

    /// @return outgoing edges (modifiable).
    std::vector<Edge<T>> &getAdj() { return adj; }

    /// @return outgoing edges (read-only).
    const std::vector<Edge<T>> &getAdj() const { return adj; }

    bool isVisited() const { return visited; }
    void setVisited(bool v) { visited = v; }

    bool isActive() const { return active; }
    void setActive(bool a) { active = a; }

    int getColor() const { return color; }
    void setColor(int c) { color = c; }

    /**
     * @brief Number of incident edges considering only @c active neighbours.
     * @complexity O(deg(v))
     */
    int activeDegree() const {
        int d = 0;
        for (const auto &e : adj) if (e.getDest()->isActive()) ++d;
        return d;
    }
};

/**
 * @brief An undirected adjacency-list edge of @ref Graph.
 *
 * The interference graph is intrinsically undirected; we store both
 * directions in each endpoint's adjacency list so that @ref Graph::addEdge
 * inserts the symmetric pair.
 */
template <class T>
class Edge {
    Vertex<T> *dest;   ///< destination endpoint
    double weight;     ///< edge weight (unused for register allocation)

public:
    Edge(Vertex<T> *d, double w) : dest(d), weight(w) {}
    Vertex<T> *getDest() const { return dest; }
    double getWeight() const { return weight; }
};

/**
 * @brief Generic adjacency-list graph parameterized over the vertex payload type.
 *
 * Vertices are uniquely identified by their @c info field.
 *
 * @tparam T payload type; must be equality-comparable.
 */
template <class T>
class Graph {
    std::vector<Vertex<T>*> vertexSet; ///< all vertices (owning)

public:
    Graph() = default;

    /// Releases owned vertices.
    ~Graph() {
        for (auto *v : vertexSet) delete v;
    }

    Graph(const Graph &) = delete;
    Graph &operator=(const Graph &) = delete;

    /**
     * @brief Locate a vertex by payload.
     * @complexity O(|V|)
     */
    Vertex<T> *findVertex(const T &in) const {
        for (auto *v : vertexSet) if (v->info == in) return v;
        return nullptr;
    }

    /**
     * @brief Insert a new vertex if none with the same payload exists.
     * @return true if inserted, false if already present.
     * @complexity O(|V|)
     */
    bool addVertex(const T &in) {
        if (findVertex(in) != nullptr) return false;
        vertexSet.push_back(new Vertex<T>(in));
        return true;
    }

    /**
     * @brief Insert an undirected edge between @p src and @p dst.
     *
     * Both endpoints must already exist. Duplicate edges are ignored.
     * @complexity O(|V| + deg(src) + deg(dst))
     */
    bool addEdge(const T &src, const T &dst, double w = 1.0) {
        Vertex<T> *s = findVertex(src);
        Vertex<T> *d = findVertex(dst);
        if (!s || !d || s == d) return false;
        for (const auto &e : s->adj) if (e.getDest() == d) return false; // dedupe
        s->adj.emplace_back(d, w);
        d->adj.emplace_back(s, w);
        return true;
    }

    /// @return the number of vertices (active or not).
    int getNumVertex() const { return static_cast<int>(vertexSet.size()); }

    /// @return all vertices.
    const std::vector<Vertex<T>*> &getVertexSet() const { return vertexSet; }
    std::vector<Vertex<T>*> &getVertexSet() { return vertexSet; }

    /// @brief Resets per-vertex transient state (active=true, color=-1, visited=false).
    void resetState() {
        for (auto *v : vertexSet) {
            v->active = true;
            v->color = -1;
            v->visited = false;
        }
    }
};
