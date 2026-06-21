#include "PoincareProjection.h"

#include <algorithm>
#include <cmath>
#include <utility>

#include "../spaces/PoincareSpace.h"

namespace gd {

PoincareProjection::PoincareProjection() : _name("poincare") {}

std::string PoincareProjection::name() const {
	return _name;
}

ProjectionResult PoincareProjection::project(
		const Embedding& emb,
		const Space& space,
		const std::vector<int32_t>& figSize,
		int32_t finalDim) const {
	if (space.name() != "hyperbolic") {
		throw ProjectionError("PoincareProjection::project: only Hyperbolic space is supported");
	}
	if (space.dimension() != emb.dimension()) {
		throw ProjectionError("PoincareProjection::project: space dimension != embedding dimension");
	}
	if (emb.dimension() != 2 || finalDim != 2) {
		throw ProjectionError("PoincareProjection::project: only H2 to 2D projection is supported");
	}
	if (figSize.size() != 2 || figSize[0] <= 0 || figSize[1] <= 0) {
		throw ProjectionError("PoincareProjection::project: figSize must contain two positive sides");
	}

	const ld radius = static_cast<ld>(std::min(figSize[0], figSize[1])) / 2.0L;
	std::vector<Pt> coords(emb.size(), Pt(2, 0.0L));
	for (int32_t i = 0; i < emb.size(); i++) {
		const Pt& x = emb.getCoord(i);
		if (!space.isValid(x)) {
			throw ProjectionError("PoincareProjection::project: invalid hyperbolic point");
		}
		const ld x0 = std::sqrtl(1.0L + x[0] * x[0] + x[1] * x[1]);
		coords[i][0] = radius * x[0] / (x0 + 1.0L);
		coords[i][1] = radius * x[1] / (x0 + 1.0L);
	}

	Embedding result = coords.empty()
		? Embedding(emb.getGraph(), finalDim)
		: Embedding(emb.getGraph(), coords);
	return ProjectionResult{
		std::move(result),
		std::make_unique<PoincareSpace>(finalDim, radius)};
}

} // namespace gd
