#include "Projection.h"

#include <vector>

namespace gd {

IdentityProjection::IdentityProjection() : _name("identity") {}

std::string IdentityProjection::name() const {
	return _name;
}

Embedding IdentityProjection::project(const Embedding& emb, const Space& space, int32_t finalDim) const {
	if (finalDim != emb.dimension()) {
		throw ProjectionError("IdentityProjection::project: final dimension != embedding dimension");
	}
	return Embedding(emb.getGraph(), emb.getCoords());
}

OrthogonalProjection::OrthogonalProjection() : _name("orthogonal") {}

std::string OrthogonalProjection::name() const {
	return _name;
}

Embedding OrthogonalProjection::project(const Embedding& emb, const Space& space, int32_t finalDim) const {
	if (space.name() != "euclidean") {
		throw ProjectionError("OrthogonalProjection::project: only Euclidean space is supported");
	}
	if (finalDim > emb.dimension()) {
		throw ProjectionError("OrthogonalProjection::project: final dimension > embedding dimension");
	}
	if (finalDim == emb.dimension()) {
		return Embedding(emb.getGraph(), emb.getCoords());
	}

	std::vector<Pt> res(emb.size());
	for (int32_t i = 0; i < emb.size(); i++) {
		const Pt& coord = emb.getCoord(i);
                res[i] = Pt(coord.begin(), coord.begin() + finalDim);
	}
	return Embedding(emb.getGraph(), res);
}

ProjectionPtr createProjection(const std::string& projName) {
	if (projName == "identity") {
		return std::make_unique<IdentityProjection>();
	}
	if (projName == "orthogonal") {
		return std::make_unique<OrthogonalProjection>();
	}
	throw ProjectionError("createProjection: unknown projection name: " + projName);
}

} // namespace gd

