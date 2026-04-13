#pragma once

#include <cstddef>
#include <vector>

#include "template.h"
#include "Errors.h"
#include "Graph.h"

namespace gd {

class EmbeddingError : public GraphDrawingError {
public:
	using GraphDrawingError::GraphDrawingError;
};

// над графом
class Embedding {
        friend class LayoutAlgorithm;
        friend class ZeroInitialPlacement;
public:
        // Embedding() = default;
        Embedding(const Graph& graph);  // найти начальную расстановку
        Embedding(const Graph& graph, int32_t dim);
        Embedding(const Graph& graph, const std::vector<Pt>& startCoords);

        void setPos(int32_t v, const Pt& pos);
        void setPosMany(const std::vector<std::pair<int32_t, Pt>>& data);

        const Pt& getCoord(int32_t v) const;

        int32_t size() const;

        int32_t dimension() const;

        const std::vector<Pt>& getCoords() const;

        const Graph& getGraph() const;

        const std::vector<std::pair<int32_t, int32_t>>& getEdges() const;
private:
        const Graph& _graph;
        int32_t curDim;
        std::vector<Pt> _coords;
};

} // namespace gd

