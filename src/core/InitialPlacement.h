#pragma once

#include <cstdint>
#include <vector>

#include "template.h"
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

	virtual void computeInitial(
		Embedding& emb,
		const Space& space,
		const std::vector<int32_t>& figSize) const = 0;
};

class ZeroInitialPlacement final : public InitialPlacementStrategy {
public:
        explicit ZeroInitialPlacement() = default;

	void computeInitial(
		Embedding& emb,
		const Space& space,
		const std::vector<int32_t>& figSize) const override;
};

class RandomInitialPlacement final : public InitialPlacementStrategy {
public:
        explicit RandomInitialPlacement() = default;

	void computeInitial(
		Embedding& emb,
		const Space& space,
		const std::vector<int32_t>& figSize) const override;
};

using InitialPlacementStrategyPtr = std::unique_ptr<InitialPlacementStrategy>;

InitialPlacementStrategyPtr createInitialPlacementStrategy(const std::string& name);

} // namespace gd

