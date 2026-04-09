#pragma once

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

#include "../core/template.h"
#include "../core/Embedding.h"
#include "../core/Errors.h"
#include "../spaces/Space.h"

namespace gd {

class LayoutError : public GraphDrawingError {
public:
	using GraphDrawingError::GraphDrawingError;
};

class LayoutAlgorithm {
public:
	explicit LayoutAlgorithm(std::string name);

	virtual ~LayoutAlgorithm() = default;

	std::string name() const;

	virtual void computeLayout(
		Embedding& emb,
		const Space& space,
		const std::vector<int32_t>& figSize) const = 0;

protected:
	std::string _name;
};

using LayoutAlgorithmPtr = std::unique_ptr<LayoutAlgorithm>;

LayoutAlgorithmPtr createLayoutAlgorithm(const std::string& name);

} // namespace gd
