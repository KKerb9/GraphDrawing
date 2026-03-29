#include "InitialPlacement.h"

namespace gd {

void ZeroInitialPlacement::computeInitial(Embedding& emb, const Space& space) const {
        Coord coord(space.dimension(), 0.0);
        emb.curDim = space.dimension();
        for (std::int32_t v = 0; v < static_cast<std::int32_t>(emb.size()); v++) emb.setPos(v, coord);
}

} // namespace gd

