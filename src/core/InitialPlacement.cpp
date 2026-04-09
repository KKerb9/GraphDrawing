#include <algorithm>
#include <random>

#include "InitialPlacement.h"

namespace gd {

void ZeroInitialPlacement::computeInitial(
		Embedding& emb,
		const Space& space,
		const std::vector<int32_t>& figSize) const {
        Pt coord(space.dimension(), 0.0);
        emb.curDim = space.dimension();
        for (int32_t v = 0; v < static_cast<int32_t>(emb.size()); v++) emb.setPos(v, coord);
}

void RandomInitialPlacement::computeInitial(
		Embedding& emb,
		const Space& space,
		const std::vector<int32_t>& figSize) const {
        int32_t n = emb.size();
        int32_t dim = space.dimension();
        if (static_cast<int32_t>(figSize.size()) != dim) {
                throw InitialPlacementError("RandomInitialPlacement: figSize != dimantion");
        }
        std::mt19937 rng(1543);
        for (int32_t i = 0; i < n; i++) {
                Pt cur(dim, 0.0);
                for (int32_t j = 0; j < dim; j++) {
                        const ld half = static_cast<ld>(figSize[j]) / 2.0L;
                        std::uniform_real_distribution<ld> dist(-half, half);
                        cur[j] = dist(rng);
                }
                emb.setPos(i, cur);
        }
}

InitialPlacementStrategyPtr createInitialPlacementStrategy(const std::string& name) {
        if (name == "zero") {
                return std::make_unique<ZeroInitialPlacement>();
        } else if (name == "random") {
                return std::make_unique<RandomInitialPlacement>();
        } else {
                throw InitialPlacementError("Unknown initial placement strategy: " + name);
        }
}

} // namespace gd

