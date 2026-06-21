#pragma once

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

#include "../core/Embedding.h"
#include "../core/Errors.h"
#include "../spaces/Space.h"

namespace gd {

class ProjectionError : public GraphDrawingError {
public:
	using GraphDrawingError::GraphDrawingError;
};

struct ProjectionResult {
	Embedding embedding;
	SpacePtr space;
};

class Projection {
public:
	virtual ~Projection() = default;

	virtual std::string name() const = 0;

	virtual ProjectionResult project(
		const Embedding& emb,
		const Space& space,
		const std::vector<int32_t>& figSize,
		int32_t finalDim) const = 0;

protected:
	static std::vector<Pt> fitToFigSize(
		const std::vector<Pt>& coords,
		const std::vector<int32_t>& figSize,
		int32_t finalDim);
};

using ProjectionPtr = std::unique_ptr<Projection>;

ProjectionPtr createProjection(const std::string& projName);

} // namespace gd
