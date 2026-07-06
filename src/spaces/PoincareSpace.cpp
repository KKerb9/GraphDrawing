#include "PoincareSpace.h"

#include <algorithm>
#include <cmath>
#include <limits>

namespace gd {

PoincareSpace::PoincareSpace(int32_t dim, ld radius)
	: _dim(dim), _radius(radius), _name("poincare") {
	if (_dim != 2) {
		throw SpaceError("PoincareSpace: only dimension 2 is supported");
	}
	if (!std::isfinite(_radius) || _radius <= 0.0L) {
		throw SpaceError("PoincareSpace: radius must be positive and finite");
	}
}

std::string PoincareSpace::name() const {
	return _name;
}

int32_t PoincareSpace::dimension() const noexcept {
	return _dim;
}

ld PoincareSpace::euclideanNorm(const Pt& p) const {
	ld sum = 0.0L;
	for (const ld x : p) sum += x * x;
	return std::sqrtl(sum);
}

Pt PoincareSpace::toSmallDisk(const Pt& p) const {
	return p / _radius;
}

Pt PoincareSpace::toDefDisk(const Pt& p) const {
	return p * _radius;
}

Pt PoincareSpace::mobiusAdd(const Pt& a, const Pt& b) const {
	ld aSq = 0.0L;
	ld bSq = 0.0L;
	ld dot = 0.0L;
	for (int32_t i = 0; i < _dim; i++) {
		aSq += a[i] * a[i];
		bSq += b[i] * b[i];
		dot += a[i] * b[i];
	}
        ld denominator = 1.0L + 2.0L * dot + aSq * bSq;
	if (denominator <= 0.0L) {
		throw SpaceError("PoincareSpace::mobiusAdd: non-positive denominator");
	}

	Pt res(_dim, 0.0L);
	ld aCoeff = 1.0L + 2.0L * dot + bSq;
	ld bCoeff = 1.0L - aSq;
	for (int32_t i = 0; i < _dim; i++) {
		res[i] = (aCoeff * a[i] + bCoeff * b[i]) / denominator;
	}
	return res;
}

ld PoincareSpace::dist(const Pt& a, const Pt& b) const {
	if (!isValid(a) || !isValid(b)) throw SpaceError("PoincareSpace::dist: invalid point");
	Pt x = toSmallDisk(a);
	Pt y = toSmallDisk(b);
	Pt delta = mobiusAdd(x * -1.0L, y);
	ld maxNorm = 1.0L - 16.0L * std::numeric_limits<ld>::epsilon();
	ld deltaNorm = std::min(euclideanNorm(delta), maxNorm);
	return 2.0L * std::atanhl(deltaNorm);
}

ld PoincareSpace::norm(const Pt& vec) const {
	if (static_cast<int32_t>(vec.size()) != _dim) {
		throw SpaceError("PoincareSpace::norm: size != dim");
	}
	for (const ld x : vec) {
		if (!std::isfinite(x)) throw SpaceError("PoincareSpace::norm: coordinate must be finite");
	}
	return euclideanNorm(vec);
}

Pt PoincareSpace::logMap(const Pt& from, const Pt& to) const {
	if (!isValid(from) || !isValid(to)) throw SpaceError("PoincareSpace::logMap: invalid point");
	ld EPS = 16.0L * std::numeric_limits<ld>::epsilon();
	Pt x = toSmallDisk(from);
	Pt y = toSmallDisk(to);
	Pt delta = mobiusAdd(x * -1.0L, y);
	ld deltaNorm = euclideanNorm(delta);
	if (deltaNorm < EPS) return Pt(_dim, 0.0L);

	ld xSq = 0.0L;
	for (const ld value : x) xSq += value * value;
	ld lambda = 2.0L / (1.0L - xSq);
	ld safeNorm = std::min(deltaNorm, 1.0L - EPS);
	ld coeff = (2.0L / lambda) * std::atanhl(safeNorm) / deltaNorm;
	return toDefDisk(delta * coeff);
}

Pt PoincareSpace::expMap(const Pt& from, const Pt& tangent) const {
	if (!isValid(from)) throw SpaceError("PoincareSpace::expMap: invalid point");
	if (static_cast<int32_t>(tangent.size()) != _dim) {
		throw SpaceError("PoincareSpace::expMap: tangent size != dim");
	}
	ld EPS = 16.0L * std::numeric_limits<ld>::epsilon();
	Pt x = toSmallDisk(from);
	Pt v = tangent / _radius;
	ld vNorm = norm(v);
	if (vNorm < EPS) return from;

	ld xSq = 0.0L;
	for (const ld value : x) xSq += value * value;
	ld lambda = 2.0L / (1.0L - xSq);
	Pt step = v / vNorm * std::tanhl(lambda * vNorm / 2.0L);
	Pt result = toDefDisk(mobiusAdd(x, step));
	return normalizePoint(result, std::vector<int32_t>(_dim, static_cast<int32_t>(2.0L * _radius)));
}

ld PoincareSpace::tangentNorm(const Pt& at, const Pt& tangent) const {
	if (!isValid(at)) throw SpaceError("PoincareSpace::tangentNorm: invalid point");
	if (static_cast<int32_t>(tangent.size()) != _dim) {
		throw SpaceError("PoincareSpace::tangentNorm: tangent size != dim");
	}
	Pt x = toSmallDisk(at);
	ld xSq = 0.0L;
	for (const ld value : x) xSq += value * value;
	ld lambda = 2.0L / (1.0L - xSq);
	return lambda * norm(tangent) / _radius;
}

Pt PoincareSpace::normalizePoint(const Pt& p, const std::vector<int32_t>& figSize) const {
	// TODO: move domain constraints out of Space
	if (static_cast<int32_t>(p.size()) != _dim) {
		throw SpaceError("PoincareSpace::normalizePoint: size != dim");
	}
	if (static_cast<int32_t>(figSize.size()) != _dim) {
		throw SpaceError("PoincareSpace::normalizePoint: figSize size != dim");
	}
	for (const ld x : p) {
		if (!std::isfinite(x)) throw SpaceError("PoincareSpace::normalizePoint: coordinate must be finite");
	}
	ld pointNorm = euclideanNorm(p);
	ld maxNorm = _radius * (1.0L - 1e-15L);
	if (pointNorm < maxNorm) return p;
	if (pointNorm == 0.0L) return p;
	return p / pointNorm * maxNorm;
}

ld PoincareSpace::volume(const std::vector<int32_t>& figSize) const {
	if (static_cast<int32_t>(figSize.size()) != _dim) {
		throw SpaceError("PoincareSpace::volume: figSize size != dim");
	}
	return std::numeric_limits<ld>::infinity();
}

bool PoincareSpace::areGeodesicSegmentsCrossing(const Pt& a, const Pt& b, const Pt& c, const Pt& d) const {
	if (!isValid(a) || !isValid(b) || !isValid(c) || !isValid(d)) {
		throw SpaceError("PoincareSpace::areGeodesicSegmentsCrossing: invalid point");
	}
	auto toKlein = [&](const Pt& point) -> Pt {
		Pt p = toSmallDisk(point);
		ld sqNorm = 0.0L;
		for (const ld x : p) sqNorm += x * x;
		return p * (2.0L / (1.0L + sqNorm));
	};
	return areEuclideanSegmentsCrossing2D(
		toKlein(a), toKlein(b), toKlein(c), toKlein(d));
}

bool PoincareSpace::isValid(const Pt& c) const {
	if (static_cast<int32_t>(c.size()) != _dim) return false;
	for (const ld x : c) {
		if (!std::isfinite(x)) return false;
	}
	return euclideanNorm(c) < _radius;
}

ld PoincareSpace::radius() const noexcept {
	return _radius;
}

} // namespace gd
