#include "OrthogonalProjection.h"

#include "../spaces/EuclideanSpace.h"

#include <utility>

namespace gd {

OrthogonalProjection::OrthogonalProjection() : _name("orthogonal") {}

std::string OrthogonalProjection::name() const {
	return _name;
}

ProjectionResult OrthogonalProjection::project(
		const Embedding& emb,
		const Space& space,
		const std::vector<int32_t>& figSize,
		int32_t finalDim) const {
	if (space.name() != "euclidean") {
		throw ProjectionError("OrthogonalProjection::project: only Euclidean space is supported");
	}
	if (finalDim > emb.dimension()) {
		throw ProjectionError("OrthogonalProjection::project: final dimension > embedding dimension");
	}
	std::vector<Pt> res(emb.size());
	for (int32_t i = 0; i < emb.size(); i++) {
		const Pt& coord = emb.getCoord(i);
		res[i] = Pt(coord.begin(), coord.begin() + finalDim);
	}
	res = fitToFigSize(res, figSize, finalDim);
	Embedding f = res.empty() ? Embedding(emb.getGraph(), finalDim) : Embedding(emb.getGraph(), res);
	return ProjectionResult{std::move(f), std::make_unique<EuclideanSpace>(finalDim)};
}

} // namespace gd
