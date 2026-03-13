#include "core/Embedding.h"

namespace gd {

Embedding::Embedding(Graph& graph) : _graph(graph), curDim(0) {
        _coords.resize(graph.vertexCount());
}

Embedding::Embedding(Graph& graph, const std::vector<Coord>& startCoords)
        : _graph(graph), curDim(static_cast<std::int32_t>(startCoords[0].size())) {
        if (static_cast<std::int32_t>(startCoords.size()) != graph.vertexCount()) {
                throw EmbeddingError("startCoords: wrong size");
        }
        _coords = startCoords;
}

void Embedding::setPos(std::int32_t v, const Coord& pos) {
        if (static_cast<std::int32_t>(pos.size()) != curDim) {
                throw EmbeddingError("setPos: coordinate dimension mismatch");
        }
        if (v < 0 || v >= static_cast<std::int32_t>(_coords.size())) {
                throw EmbeddingError("setPos: vertex index out of range");
        }
        _coords[v] = pos;
}

void Embedding::setPosMany(const std::vector<std::pair<std::int32_t, Coord>>& data) {
        for (const auto& [v, pos] : data) {
                if (static_cast<std::int32_t>(pos.size()) != curDim) {
                        throw EmbeddingError("setPosMany: coordinate dimension mismatch");
                }
                if (v < 0 || v >= static_cast<std::int32_t>(_coords.size())) {
                        throw EmbeddingError("setPosMany: vertex index out of range");
                }
                _coords[v] = pos;
        }
}

const Coord& Embedding::getCoord(std::int32_t v) const {
        if (v < 0 || v >= static_cast<std::int32_t>(_coords.size())) {
                throw EmbeddingError("getCoord: vertex index out of range");
        }
        return _coords[v];
}

std::int32_t Embedding::size() const {
        return static_cast<std::int32_t>(_coords.size());
}

std::int32_t Embedding::dimension() const {
        return static_cast<std::int32_t>(curDim);
}

const std::vector<Coord>& Embedding::all() const {
        return _coords;
}

} // namespace gd
