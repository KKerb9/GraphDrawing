#pragma once

#include <cstdint>
#include <vector>

#include "core/Errors.h"

namespace gd {

class GraphError : public GraphDrawingError {
public:
	using GraphDrawingError::GraphDrawingError;
};

class Graph {
public:
        Graph() = default;

        explicit Graph(std::int32_t vertexCount);

        std::int32_t vertexCount() const;
        std::int32_t edgeCount() const;

        // bool hasVertex(std::int32_t v) const noexcept;

        void addVertex(std::int32_t v);

        void addEdge(std::int32_t u, std::int32_t v);

	const std::vector<std::int32_t>& neighbors(std::int32_t v) const;
private:
	std::vector<std::vector<std::int32_t>> matr;
	std::int32_t _m{0};
};

} // namespace gd

