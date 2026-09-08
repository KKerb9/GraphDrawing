#include "EuclideanSpace.h"

#include <algorithm>
#include <cmath>

namespace gd {

EuclideanSpace::EuclideanSpace(int32_t dim) : _dim(dim), _name("euclidean") {}

std::string EuclideanSpace::name() const {
        return _name;
}

int32_t EuclideanSpace::dimension() const noexcept {
        return _dim;
}

ld EuclideanSpace::dist(const Pt& a, const Pt& b) const {
        if (!isValid(a) || !isValid(b)) throw SpaceError("dist: size != dim");
        ld summ = 0.0;
        for (int32_t i = 0; i < _dim; i++) {
            summ += (a[i] - b[i]) * (a[i] - b[i]);
        }
        return std::sqrt(summ);
}

bool EuclideanSpace::isValid(const Pt& c) const {
        return static_cast<int32_t>(c.size()) == _dim;
        // throw SpaceError("coordinate dimension mismatch");
}

ld EuclideanSpace::norm(const Pt& vec) const {
        if (!isValid(vec)) throw SpaceError("norm: size != dim");
        ld summ = 0.0;
        for (const ld x : vec) {
                summ += x * x;
        }
        return std::sqrtl(summ);
}

Pt EuclideanSpace::logMap(const Pt& from, const Pt& to) const {
        if (!isValid(from) || !isValid(to)) throw SpaceError("logMap: size != dim");
        return to - from;
}

Pt EuclideanSpace::expMap(const Pt& from, const Pt& tangent) const {
        if (!isValid(from) || !isValid(tangent)) throw SpaceError("expMap: size != dim");
        return from + tangent;
}

ld EuclideanSpace::tangentNorm(const Pt& at, const Pt& tangent) const {
        if (!isValid(at) || !isValid(tangent)) throw SpaceError("tangentNorm: size != dim");
        return norm(tangent);
}

Pt EuclideanSpace::normalizePoint(const Pt& p, const std::vector<int32_t>& figSize) const {
        if (!isValid(p)) throw SpaceError("normalizePoint: size != dim");
        if (static_cast<int32_t>(figSize.size()) != _dim) {
                throw SpaceError("normalizePoint: figSize size != dim");
        }
        Pt res = p;
        for (int32_t i = 0; i < _dim; i++) {
                res[i] = std::min((ld)figSize[i] / 2, std::max(-(ld)figSize[i] / 2, res[i]));
        }
        return res;
}

ld EuclideanSpace::volume(const std::vector<int32_t>& figSize) const {
        if (static_cast<int32_t>(figSize.size()) != _dim) {
                throw SpaceError("volume: figSize size != dim");
        }
        // std::vector<std::vector<ld>> gram(_dim, std::vector<ld>(_dim, 0.0L));
        ld det = 1.0;
        for (int32_t i = 0; i < _dim; i++) {
                det *= (ld)figSize[i] * figSize[i];
                // gram[i][i] = (ld)figSize[i] * figSize[i];
        }
        // ld det = Space::determinantBareiss(gram);
        // det = std::max(det, 0.0L);
        return std::sqrtl(det);
}

bool EuclideanSpace::areGeodesicSegmentsCrossing(
                const Pt& a,
                const Pt& b,
                const Pt& c,
                const Pt& d) const {
        if (_dim != 2) {
                throw SpaceError("areGeodesicSegmentsCrossing: Euclidean dimension != 2");
        }
        if (!isValid(a) || !isValid(b) || !isValid(c) || !isValid(d)) {
                throw SpaceError("areGeodesicSegmentsCrossing: size != dim");
        }
        return areEuclideanSegmentsCrossing2D(a, b, c, d);
}

} // namespace gd
