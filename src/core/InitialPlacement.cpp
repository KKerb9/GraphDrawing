#include <algorithm>
#include <random>

#include "InitialPlacement.h"

namespace gd {

void ZeroInitialPlacement::computeInitial(Embedding& emb, const Space& space) const {
        Coord coord(space.dimension(), 0.0);
        emb.curDim = space.dimension();
        for (int32_t v = 0; v < static_cast<int32_t>(emb.size()); v++) emb.setPos(v, coord);
}

void RandomInitialPlacement::computeInitial(Embedding& emb, const Space& space) const {
        int32_t n = emb.size();
        int32_t dim = space.dimension();
        std::mt19937 rng(1543);
        std::uniform_real_distribution<double> dist(std::min(-100, -n - 1), std::max(n + 1, 100));
        for (int32_t i = 0; i < n; i++) {
                Coord cur(dim, 0.0);
                for (int32_t j = 0; j < dim; j++) {
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

