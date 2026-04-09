#pragma once

#include <cstdint>
#include <vector>

#include "Layout.h"

namespace gd {

class RandomLayout : public LayoutAlgorithm {
public:
	RandomLayout();

	void computeLayout(
		Embedding& emb,
		const Space& space,
		const std::vector<int32_t>& figSize) const override;
};

} // namespace gd
