#include "Embedding.h"

namespace gd {

Embedding::Embedding(Graph& graph) : _graph(graph), curDim(0) {
        _coords.resize(graph.vertexCount());
}

Embedding::Embedding(Graph& graph, int32_t dim) : _graph(graph), curDim(dim) {
        _coords.resize(graph.vertexCount());
}

Embedding::Embedding(Graph& graph, const std::vector<Pt>& startCoords)
        : _graph(graph), curDim(static_cast<int32_t>(startCoords[0].size())) {
        if (static_cast<int32_t>(startCoords.size()) != graph.vertexCount()) {
                throw EmbeddingError("startCoords: wrong size");
        }
        _coords = startCoords;
}

void Embedding::setPos(int32_t v, const Pt& pos) {
        if (static_cast<int32_t>(pos.size()) != curDim) {
                std::cerr << pos.size() << ' ' << curDim << '\n';
                throw EmbeddingError("setPos: coordinate dimension mismatch");
        }
        if (v < 0 || v >= static_cast<int32_t>(_coords.size())) {
                throw EmbeddingError("setPos: vertex index out of range");
        }
        _coords[v] = pos;
}

void Embedding::setPosMany(const std::vector<std::pair<int32_t, Pt>>& data) {
        for (const auto& [v, pos] : data) {
                if (static_cast<int32_t>(pos.size()) != curDim) {
                        throw EmbeddingError("setPosMany: coordinate dimension mismatch");
                }
                if (v < 0 || v >= static_cast<int32_t>(_coords.size())) {
                        throw EmbeddingError("setPosMany: vertex index out of range");
                }
                _coords[v] = pos;
        }
}

const Pt& Embedding::getCoord(int32_t v) const {
        if (v < 0 || v >= static_cast<int32_t>(_coords.size())) {
                throw EmbeddingError("getCoord: vertex index out of range");
        }
        return _coords[v];
}

int32_t Embedding::size() const {
        return static_cast<int32_t>(_coords.size());
}

int32_t Embedding::dimension() const {
        return static_cast<int32_t>(curDim);
}

const std::vector<Pt>& Embedding::getCoords() const {
        return _coords;
}

const Graph& Embedding::getGraph() const {
        return _graph;
}

const std::vector<std::pair<int32_t, int32_t>>& Embedding::getEdges() const {
        return _graph.edges();
}

} // namespace gd
