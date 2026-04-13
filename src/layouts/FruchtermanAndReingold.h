#pragma once

#include <cstdint>
#include <vector>

#include "Layout.h"

namespace gd {

class FruchtermanAndReingoldLayout : public LayoutAlgorithm {
public:
        FruchtermanAndReingoldLayout(int32_t ITERS, ld C);

        void computeLayout(
		Embedding& emb,
		const Space& space,
		const std::vector<int32_t>& figSize) const override;
private:
	int32_t ITERS;
	ld C;
};

} // namespace gd
