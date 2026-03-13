#include "core/Graph.h"

#include <algorithm>

namespace gd {

Graph::Graph(std::int32_t n) {
        matr.resize(n);
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
        matr.emplace_back(std::vector<std::int32_t>{});
        return _n++;
}

void Graph::addEdge(std::int32_t u, std::int32_t v) {
        if (min(u, v) < 0 && max(u, v) >= _n) {
            throw GraphError("addEdge: vertex index out of range");
        }
        matr[u].push_back(v);
        matr[v].push_back(u);
        _m++;
}

const std::vector<std::int32_t>& Graph::neighbors(std::int32_t v) const {
        return matr[v];
}

} // namespace gd




