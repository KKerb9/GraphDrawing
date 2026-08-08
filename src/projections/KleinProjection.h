#pragma once

#include <cstdint>
#include <random>
#include <vector>

#include "Projection.h"

namespace gd {

enum class KleinProjectionMode {
	Orthogonal,
	BestView,
};

class KleinProjection : public Projection {
public:
	KleinProjection(KleinProjectionMode mode, uint32_t seed, int32_t candidates);

	std::string name() const override;

	ProjectionResult project(
		const Embedding& emb,
		const Space& space,
		const std::vector<int32_t>& figSize,
		int32_t finalDim) const override;

private:
	std::vector<Pt> toKleinCoords(const Embedding& emb, const Space& space) const;
	std::vector<Pt> projectWithCamera(const std::vector<Pt>& kleinCoords, const std::vector<Pt>& camera, ld radius) const;
	std::vector<Pt> orthogonalCamera(int32_t dim) const;
	std::vector<Pt> randomCamera(int32_t dim, std::mt19937_64& rng) const;

	KleinProjectionMode _mode;
	uint32_t _seed;
	int32_t _candidates;
	std::string _name;
};

} // namespace gd
