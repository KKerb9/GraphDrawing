#pragma once

#include "../core/Embedding.h"
#include "../core/Errors.h"
#include "../spaces/Space.h"

namespace gd {

class MetricsError : public GraphDrawingError {
public:
	using GraphDrawingError::GraphDrawingError;
};

struct Metrics {
        ld stress{0.0};
        ld edgeLengthVariance{0.0};
};

class MetricsCalculator {
public:
        Metrics compute(const Embedding& emb, const Space& space) const;
};

} // namespace gd

