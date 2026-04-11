#pragma once

#include <string>

#include "Config.h"
#include "../core/Embedding.h"
#include "../core/Errors.h"
#include "../metrics/Metrics.h"

namespace gd {

class EmbeddingWriterError : public GraphDrawingError {
public:
	using GraphDrawingError::GraphDrawingError;
};

void writeEmbeddingJson(
	const Config& cfg,
	const Embedding& result,
	const Metrics& metrics);
        
} // namespace gd

