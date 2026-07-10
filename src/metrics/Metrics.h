#pragma once

#include <cstdint>
#include <vector>

#include "../core/Errors.h"
#include "../core/template.h"
#include "../core/Embedding.h"
#include "../spaces/Space.h"

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
        ld minEdgeVertexDist;
};

Metrics computeMetrics(const Embedding& emb, const Space& space);

} // namespace gd
