#include "RandomLayout.h"

#include <cstdint>
#include <random>

namespace gd {

RandomLayout::RandomLayout()
	: LayoutAlgorithm("random") {}

void RandomLayout::computeLayout(
		Embedding& emb,
		const Space& space,
		const std::vector<int32_t>& figSize) const {
        const int32_t dim = space.dimension();
        
        if (emb.dimension() != dim) {
                throw LayoutError("RandomLayout: embedding dimension mismatch");
        }
        if (static_cast<int32_t>(figSize.size()) != dim) {
                throw LayoutError("RandomLayout: figSize dimension mismatch");
        }
        std::random_device rd;
        std::mt19937 rng(rd());

        for (int32_t v = 0; v < static_cast<int32_t>(emb.size()); v++) {
                Pt c(dim, 0.0);
                for (int32_t i = 0; i < dim; i++) {
                        const ld half = static_cast<ld>(figSize[i]) / 2.0L;
                        std::uniform_real_distribution<ld> dist(-half, half);
                        c[i] = dist(rng);
                }
                emb.setPos(v, c);
        }
}

} // namespace gd
