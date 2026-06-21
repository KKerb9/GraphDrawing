#pragma once

#include <cstdint>
#include <vector>

#include "Layout.h"

namespace gd {

class FruchtermanAndReingoldLayout : public LayoutAlgorithm {
public:
	FruchtermanAndReingoldLayout();
        FruchtermanAndReingoldLayout(int32_t ITERS, ld C);

        void computeLayout(
		Embedding& emb,
		const Space& space,
		const BorderPolicy& borderPolicy
	) const override;

        void computeLayoutTest(
                Embedding& emb,
                const Space& space,
                const BorderPolicy& borderPolicy
        ) const;

        void computeLayoutEuclideanOld(
                Embedding& emb,
                const Space& space,
                const std::vector<int32_t>& figSize
        ) const;
private:
	int32_t ITERS;
	ld C;
};

} // namespace gd
