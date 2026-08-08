#include "KleinSpace.h"

#include <cmath>

namespace gd {

KleinSpace::KleinSpace(int32_t dim, ld radius)
	: _dim(dim), _radius(radius), _name("klein") {
	if (_dim != 2) {
		throw SpaceError("KleinSpace: only dimension 2 is supported");
	}
	if (!std::isfinite(_radius) || _radius <= 0.0L) {
		throw SpaceError("KleinSpace: radius must be positive and finite");
	}
}

std::string KleinSpace::name() const {
	return _name;
}

int32_t KleinSpace::dimension() const noexcept {
	return _dim;
}

ld KleinSpace::dist(const Pt& a, const Pt& b) const {
	if (!isValid(a) || !isValid(b)) throw SpaceError("KleinSpace::dist: invalid point");
	return norm(b - a);
}

ld KleinSpace::norm(const Pt& vec) const {
	if (static_cast<int32_t>(vec.size()) != _dim) {
		throw SpaceError("KleinSpace::norm: size != dim");
	}
	ld sum = 0.0L;
	for (ld x : vec) {
		if (!std::isfinite(x)) throw SpaceError("KleinSpace::norm: coordinate must be finite");
		sum += x * x;
	}
	return std::sqrtl(sum);
}

Pt KleinSpace::logMap(const Pt& from, const Pt& to) const {
	if (!isValid(from) || !isValid(to)) throw SpaceError("KleinSpace::logMap: invalid point");
	return to - from;
}

Pt KleinSpace::expMap(const Pt& from, const Pt& tangent) const {
	if (!isValid(from) || static_cast<int32_t>(tangent.size()) != _dim) {
		throw SpaceError("KleinSpace::expMap: invalid point or tangent");
	}
	return clampToDisk(from + tangent);
}

ld KleinSpace::tangentNorm(const Pt& at, const Pt& tangent) const {
	if (!isValid(at)) throw SpaceError("KleinSpace::tangentNorm: invalid point");
	return norm(tangent);
}

Pt KleinSpace::normalizePoint(const Pt& p, const std::vector<int32_t>& figSize) const {
	if (static_cast<int32_t>(figSize.size()) != _dim) {
		throw SpaceError("KleinSpace::normalizePoint: figSize size != dim");
	}
	return clampToDisk(p);
}

ld KleinSpace::volume(const std::vector<int32_t>& figSize) const {
	if (static_cast<int32_t>(figSize.size()) != _dim) {
		throw SpaceError("KleinSpace::volume: figSize size != dim");
	}
	return std::acosl(-1.0L) * _radius * _radius;
}

bool KleinSpace::areGeodesicSegmentsCrossing(
		const Pt& a,
		const Pt& b,
		const Pt& c,
		const Pt& d) const {
	if (!isValid(a) || !isValid(b) || !isValid(c) || !isValid(d)) {
		throw SpaceError("KleinSpace::areGeodesicSegmentsCrossing: invalid point");
	}
	return areEuclideanSegmentsCrossing2D(a, b, c, d);
}

bool KleinSpace::isValid(const Pt& c) const {
	if (static_cast<int32_t>(c.size()) != _dim) return false;
	ld sum = 0.0L;
	for (ld x : c) {
		if (!std::isfinite(x)) return false;
		sum += x * x;
	}
	return std::sqrtl(sum) < _radius;
}

ld KleinSpace::radius() const noexcept {
	return _radius;
}

Pt KleinSpace::clampToDisk(const Pt& p) const {
	if (static_cast<int32_t>(p.size()) != _dim) {
		throw SpaceError("KleinSpace::clampToDisk: size != dim");
	}
	ld pointNorm = norm(p);
	ld maxNorm = _radius * (1.0L - 1e-12L);
	if (pointNorm <= maxNorm) return p;
	return p / pointNorm * maxNorm;
}

} // namespace gd
