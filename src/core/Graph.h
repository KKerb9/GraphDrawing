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

	explicit Graph(int32_t vertexCount);

	int32_t vertexCount() const;
	int32_t edgeCount() const;

	int32_t addVertex();

	void addEdge(int32_t u, int32_t v);

	const std::vector<int32_t>& neighbors(int32_t v) const;

	const std::vector<std::pair<int32_t, int32_t>>& edges() const;
private:
	std::vector<std::vector<int32_t>> _matr;
	std::vector<std::pair<int32_t, int32_t>> _edges;
	int32_t _n;
	int32_t _m;
};

} // namespace gd

