#pragma once

#include <cstdint>
#include <memory>
#include <random>
#include <string>
#include <vector>

#include "BorderPolicy.h"
#include "Errors.h"
#include "template.h"
#include "../spaces/Space.h"

namespace gd {

class PolyBorderPolicy : public BorderPolicy {
public:
	PolyBorderPolicy(const BorderPolicyInteractiveParams& params, int32_t dim, uint32_t seed);

	Pt force(const Space& space, const Pt& point) const override;

	Pt normalizePoint(const Space& space, const Pt& point) const override;

	Pt randomPoint(const Space& space) const override;

	ld domainVolume(const Space& space) const override;

private:
	int32_t _dim;
	std::vector<int32_t> _figSize;
	mutable std::mt19937 _rng;
};
} // namespace gd
