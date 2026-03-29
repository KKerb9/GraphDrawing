#pragma once

#include <cstdint>
#include <vector>

#include "Errors.h"

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

	std::int32_t addVertex();

	void addEdge(std::int32_t u, std::int32_t v);

	const std::vector<std::int32_t>& neighbors(std::int32_t v) const;

	const std::vector<std::pair<std::int32_t, std::int32_t>>& edges() const;
private:
	std::vector<std::vector<std::int32_t>> _matr;
	std::vector<std::pair<std::int32_t, std::int32_t>> _edges;
	std::int32_t _n;
	std::int32_t _m;
};

} // namespace gd

