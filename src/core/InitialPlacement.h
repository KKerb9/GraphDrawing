#pragma once

#include "Embedding.h"
#include "Errors.h"
#include "../spaces/Space.h"

namespace gd {

class InitialPlacementError : public GraphDrawingError {
public:
	using GraphDrawingError::GraphDrawingError;
};

class InitialPlacementStrategy {
public:
	virtual ~InitialPlacementStrategy() = default;

	virtual void computeInitial(Embedding& emb, const Space& space) const = 0;
};

class ZeroInitialPlacement final : public InitialPlacementStrategy {
public:
        explicit ZeroInitialPlacement() = default;

	void computeInitial(Embedding& emb, const Space& space) const override;
};

} // namespace gd

