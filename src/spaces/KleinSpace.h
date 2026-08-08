#pragma once

#include "Space.h"

namespace gd {

class KleinSpace : public Space {
public:
	KleinSpace(int32_t dim, ld radius);

	std::string name() const override;

	int32_t dimension() const noexcept override;

	ld dist(const Pt& a, const Pt& b) const override;

	ld norm(const Pt& vec) const override;

	Pt logMap(const Pt& from, const Pt& to) const override;

	Pt expMap(const Pt& from, const Pt& tangent) const override;

	ld tangentNorm(const Pt& at, const Pt& tangent) const override;

	Pt normalizePoint(const Pt& p, const std::vector<int32_t>& figSize) const override;

	ld volume(const std::vector<int32_t>& figSize) const override;

	bool areGeodesicSegmentsCrossing(const Pt& a, const Pt& b, const Pt& c, const Pt& d) const override;

	bool isValid(const Pt& c) const override;

	ld radius() const noexcept;

private:
	Pt clampToDisk(const Pt& p) const;

	int32_t _dim;
	ld _radius;
	std::string _name;
};

} // namespace gd
