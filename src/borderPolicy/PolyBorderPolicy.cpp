#include "PolyBorderPolicy.h"

#include <cmath>
#include <limits>
#include <random>

#include "../io/Config.h"

namespace gd {

namespace {

constexpr int32_t kDefaultPolySide = 20;

std::vector<int32_t> makeFigSize(const BorderPolicyInteractiveParams& params, int32_t dim) {
	if (params.sideSizes.empty()) {
		return std::vector<int32_t>(dim, kDefaultPolySide);
	}
	if (static_cast<int32_t>(params.sideSizes.size()) != dim) {
		throw BorderPolicyError("PolyBorderPolicy: sideSizes size != dim");
	}

	std::vector<int32_t> figSize(dim);
	for (int32_t i = 0; i < dim; i++) {
		ld side = params.sideSizes[i];
		if (side <= 0.0L) {
			throw BorderPolicyError("PolyBorderPolicy: side size must be positive");
		}
		if (std::floorl(side) != side || side > static_cast<ld>(std::numeric_limits<int32_t>::max())) {
			throw BorderPolicyError("PolyBorderPolicy: side size must be int32");
		}
		figSize[i] = side;
	}
	return figSize;
}

} // namespace

PolyBorderPolicy::PolyBorderPolicy(const BorderPolicyInteractiveParams& params, int32_t dim, uint32_t seed)
	: BorderPolicy("poly"), _dim(dim), _figSize(makeFigSize(params, dim)), _rng(seed) {}

Pt PolyBorderPolicy::force(const Space& space, const Pt& point) const {
	return Pt(space.dimension(), 0.0L);
}

Pt PolyBorderPolicy::normalizePoint(const Space& space, const Pt& point) const {
        // TODO: normalizePoint получился у двух сущностей
	return space.normalizePoint(point, _figSize);
}

Pt PolyBorderPolicy::randomPoint(const Space& space) const {
	if (space.dimension() != _dim) {
		throw BorderPolicyError("PolyBorderPolicy: space dimension mismatch");
	}
	Pt res(_dim, 0.0L);
	for (int32_t i = 0; i < _dim; i++) {
		std::uniform_real_distribution<ld> dist(
			-static_cast<ld>(_figSize[i]) / 2.0L,
			static_cast<ld>(_figSize[i]) / 2.0L);
		res[i] = dist(_rng);
	}
	return res;
}

ld PolyBorderPolicy::domainVolume(const Space& space) const {
	return space.volume(_figSize);
}

} // namespace gd
