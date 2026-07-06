#include "BorderPolicy.h"
#include "PolyBorderPolicy.h"

#include <iostream>
#include <random>
#include <utility>

#include "../io/Config.h"

namespace gd {

BorderPolicy::BorderPolicy(std::string name)
	: _name(std::move(name)) {}

std::string BorderPolicy::name() const {
	return _name;
}

DefaultBorderPolicy::DefaultBorderPolicy(
		const BorderPolicyInteractiveParams& params,
		uint32_t seed)
	: BorderPolicy("default"), _rng(seed) {
	(void)params;
}

Pt DefaultBorderPolicy::force(const Space& space, const Pt& point) const {
	return Pt(space.dimension(), 0.0L);
}

Pt DefaultBorderPolicy::normalizePoint(const Space& space, const Pt& point) const {
	return point;
}

Pt DefaultBorderPolicy::randomPoint(const Space& space) const {
	int32_t dim = space.dimension();
	Pt p(dim, 0.0L);
	std::uniform_real_distribution<ld> dist(-10.0L, 10.0L);
	for (int32_t i = 0; i < dim; i++) {
		p[i] = dist(_rng);
	}
	return p;
}

ld DefaultBorderPolicy::domainVolume(const Space& space) const {
        return -1;
	// int32_t dim = space.dimension();
	// std::vector<int32_t> figSize(dim, 20);
	// for (int32_t i = 0; i < dim && i < static_cast<int32_t>(_defaultBoxSize.size()); i++) {
	// 	figSize[i] = _defaultBoxSize[i];
	// }
	// return space.volume(figSize);
}

BorderPolicyPtr createBorderPolicy(const std::string& name, int32_t dim, uint32_t seed) {
	if (name == "default") {
		BorderPolicyInteractiveParams p = readBorderPolicyInteractiveParams(name, dim, std::cin, std::cerr);
		return std::make_unique<DefaultBorderPolicy>(p, seed);
	} else if (name == "poly") {
		BorderPolicyInteractiveParams p = readBorderPolicyInteractiveParams(name, dim, std::cin, std::cerr);
		return std::make_unique<PolyBorderPolicy>(p, dim, seed);
	}
	throw BorderPolicyError("Unknown border policy: " + name);
}

} // namespace gd
