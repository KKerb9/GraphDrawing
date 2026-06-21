#include "RandomLayout.h"

#include <cstdint>
#include <random>

namespace gd {

RandomLayout::RandomLayout()
	: LayoutAlgorithm("random") {}

void RandomLayout::computeLayout(
		Embedding& emb,
		const Space& space,
		const BorderPolicy& borderPolicy) const {
        const int32_t dim = space.dimension();
        
        if (emb.dimension() != dim) {
                throw LayoutError("RandomLayout: embedding dimension mismatch");
        }

        for (int32_t v = 0; v < static_cast<int32_t>(emb.size()); v++) {
                emb.setPos(v, borderPolicy.randomPoint(space));
        }
}

} // namespace gd
