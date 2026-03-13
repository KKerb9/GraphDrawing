#pragma once

#include <string>

#include "core/Embedding.h"
#include "core/Errors.h"
#include "core/Projection.h"
#include "metrics/Metrics.h"

namespace gd {

class EmbeddingWriterError : public GraphDrawingError {
public:
	using GraphDrawingError::GraphDrawingError;
};

void writeEmbeddingJson(
        const std::string& outputPath,
        const std::string& graphName,
        const std::string& algoName,
        const std::string& spaceName,
        const std::string& initialPlacementName,
        const Projection& proj,
        const Embedding& emb,
        const Metrics& metrics);
        
} // namespace gd

