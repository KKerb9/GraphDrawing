#include "IdentityProjection.h"

#include "../spaces/EuclideanSpace.h"

#include <utility>

namespace gd {

IdentityProjection::IdentityProjection() : _name("identity") {}

std::string IdentityProjection::name() const {
	return _name;
}

ProjectionResult IdentityProjection::project(
		const Embedding& emb,
		const Space& space,
		const std::vector<int32_t>& figSize,
		int32_t finalDim) const {
	if (space.name() != "euclidean") {
		throw ProjectionError("IdentityProjection::project: only Euclidean space is supported");
	}
	if (finalDim != emb.dimension()) {
		throw ProjectionError("IdentityProjection::project: final dimension != embedding dimension");
	}
	std::vector<Pt> res = fitToFigSize(emb.getCoords(), figSize, finalDim);
	Embedding f = res.empty() ? Embedding(emb.getGraph(), finalDim) : Embedding(emb.getGraph(), res);
	return ProjectionResult{std::move(f), std::make_unique<EuclideanSpace>(finalDim)};
}

} // namespace gd
