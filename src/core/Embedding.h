#pragma once

#include <cstddef>
#include <vector>

#include "core/Errors.h"

namespace gd {

using Coord = std::vector<double>;

struct Vec2 {
        double x{};
        double y{};
};

struct Vec3 {
        double x{};
        double y{};
        double z{};
};

class EmbeddingError : public GraphDrawingError {
public:
	using GraphDrawingError::GraphDrawingError;
};

// над графом
class Embedding {
public:
        Embedding() = default;
        Embedding(Graph& graph);  // найти начальную расстановку
        Embedding(Graph& graph, const std::vector<Coord>& startCoords);

        void setPos(std::int32_t v, const Coord& pos);
        void setPosMany(const std::vector<std::pair<std::int32_t, Coord>>& data);

        const Coord& getCoord(std::int32_t v) const;

        std::int32_t size() const;

        std::int32_t dimension() const;

        const std::vector<Coord>& all() const;

private:
        std::int32_t curDim;
        std::vector<Coord> _coords;
        Graph& _graph;
};

} // namespace gd

