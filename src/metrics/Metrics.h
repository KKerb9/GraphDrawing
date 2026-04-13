#pragma once

#include <cstdint>
#include <vector>

#include "../core/Errors.h"
#include "../core/template.h"
#include "../spaces/Space.h"
#include "../core/Embedding.h"

namespace gd {

class MetricsError : public GraphDrawingError {
public:
	using GraphDrawingError::GraphDrawingError;
};

struct Metrics {
	ld volume;
	ld minVertexDist;
	ld maxVertexDist;
	ld avgVertexDist;
	int32_t edgeCrossings;
	ld minAngle;
	ld maxAngle;
	ld density;
};

Metrics computeMetrics(const Embedding& emb, const Space& space, const std::vector<int32_t>& figSize);

} // namespace gd
