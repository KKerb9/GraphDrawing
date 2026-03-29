#include "Graph.h"

namespace gd {

Graph::Graph(std::int32_t n) {
        _matr.resize(n);
        _n = n;
        _m = 0;
}

std::int32_t Graph::vertexCount() const {
        return _n;
}

std::int32_t Graph::edgeCount() const {
        return _m;
}

std::int32_t Graph::addVertex() {
        _matr.emplace_back(std::vector<std::int32_t>{});
        return _n++;
}

void Graph::addEdge(std::int32_t u, std::int32_t v) {
        if (std::min(u, v) < 0 || std::max(u, v) >= _n) {
            throw GraphError("addEdge: vertex index out of range");
        }
        _matr[u].push_back(v);
        _matr[v].push_back(u);
        _edges.emplace_back(u, v);
        _m++;
}

const std::vector<std::int32_t>& Graph::neighbors(std::int32_t v) const {
        return _matr[v];
}

const std::vector<std::pair<std::int32_t, std::int32_t>>& Graph::edges() const {
        return _edges;
}

} // namespace gd




