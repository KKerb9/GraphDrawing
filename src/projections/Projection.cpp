#include "Projection.h"

#include "IdentityProjection.h"
#include "KleinProjection.h"
#include "OrthogonalProjection.h"
#include "PoincareProjection.h"

#include <algorithm>
#include <cmath>
#include <limits>
#include <vector>

namespace gd {

std::vector<Pt> Projection::fitToFigSize(
		const std::vector<Pt>& coords,
		const std::vector<int32_t>& figSize,
		int32_t finalDim) {
	if (finalDim <= 0) {
		throw ProjectionError("fitToFigSize: final dimension must be positive");
	}
	if (static_cast<int32_t>(figSize.size()) != finalDim) {
		throw ProjectionError("fitToFigSize: figSize size != final dimension");
	}
	for (const int32_t side : figSize) {
		if (side <= 0) {
			throw ProjectionError("fitToFigSize: figSize side must be positive");
		}
	}
	if (coords.empty()) {
		return coords;
	}

	Pt mns(finalDim, std::numeric_limits<ld>::infinity());
	Pt mxs(finalDim, -std::numeric_limits<ld>::infinity());
	for (const Pt& p : coords) {
		if (static_cast<int32_t>(p.size()) != finalDim) {
			throw ProjectionError("fitToFigSize: coordinate dimension mismatch");
		}
		for (int32_t i = 0; i < finalDim; i++) {
			if (!std::isfinite(p[i])) {
				throw ProjectionError("fitToFigSize: coordinate must be finite");
			}
			mns[i] = std::min(mns[i], p[i]);
			mxs[i] = std::max(mxs[i], p[i]);
		}
	}

	ld scale = std::numeric_limits<ld>::infinity();
	for (int32_t i = 0; i < finalDim; i++) {
		ld len = mxs[i] - mns[i];
		if (len > 0) {
			scale = std::min(scale, static_cast<ld>(figSize[i]) / len);
		}
	}
	if (!std::isfinite(scale)) {
		scale = 1;
	}

	std::vector<Pt> res = coords;
	for (Pt& p : res) {
		for (int32_t i = 0; i < finalDim; i++) p[i] = (p[i] - (mns[i] + mxs[i]) / 2) * scale;
	}
	return res;
}

ProjectionPtr createProjection(const std::string& projName, uint32_t seed, int32_t cameraCandidates) {
	if (projName == "identity") {
		return std::make_unique<IdentityProjection>();
	}
	if (projName == "orthogonal") {
		return std::make_unique<OrthogonalProjection>();
	}
	if (projName == "poincare") {
		return std::make_unique<PoincareProjection>();
	}
	if (projName == "kleinOrthogonal") {
		return std::make_unique<KleinProjection>(KleinProjectionMode::Orthogonal, seed, cameraCandidates);
	}
	if (projName == "kleinBestView") {
		return std::make_unique<KleinProjection>(KleinProjectionMode::BestView, seed, cameraCandidates);
	}
	throw ProjectionError("createProjection: unknown projection name: " + projName);
}

} // namespace gd
