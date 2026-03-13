#include "layouts/RandomLayout.h"

#include <cstdint>
#include <random>

namespace gd {

RandomLayout::RandomLayout() : LayoutAlgorithm("random") {}

void RandomLayout::computeLayout(Embedding& emb, const Space& space) const {
        const std::int32_t dim = space.dimension();
        
        if (emb.dimension() != dim) {
                throw LayoutError("RandomLayout: embedding dimension mismatch");
        }
        std::random_device rd;
        std::mt19937 rng(rd());
        std::uniform_real_distribution<double> dist(-100.0, 100.0);

        for (std::int32_t v = 0; v < static_cast<std::int32_t>(emb.size()); v++) {
                Coord c(dim, 0.0);
                for (std::int32_t i = 0; i < dim; i++) {
                        c[i] = dist(rng);
                }
                emb.setPos(v, c);
        }
}

} // namespace gd
