#include "HyperbolicSpace.h"

#include <cmath>
#include <algorithm>

namespace gd {

HyperbolicSpace::HyperbolicSpace(int32_t dim) : _dim(dim), _name("hyperbolic") {}

std::string HyperbolicSpace::name() const {
        return _name;
}

int32_t HyperbolicSpace::dimension() const noexcept {
        return _dim;
}

ld HyperbolicSpace::lorentzProd(const Pt& a, const Pt& b) const {
        if (static_cast<int32_t>(a.size()) != _dim + 1 || static_cast<int32_t>(b.size()) != _dim + 1) {
                throw SpaceError("lorentzProd: size != dim + 1");
        }
        ld res = -a[0] * b[0];
        for (int32_t i = 1; i <= _dim; i++) {
                res += a[i] * b[i];
        }
        return res;
}

Pt HyperbolicSpace::lift(const Pt& x) const {
        if (!isValid(x)) throw SpaceError("lift: size != dim");
        ld sqNorm = 0.0L;
        for (int32_t i = 0; i < _dim; i++) {
                sqNorm += x[i] * x[i];
        }

        Pt res(_dim + 1, 0.0L);
        res[0] = std::sqrtl(1.0L + sqNorm);
        for (int32_t i = 0; i < _dim; i++) {
                res[i + 1] = x[i];
        }
        return res;
}

Pt HyperbolicSpace::defaultToTangent(const Pt& def) const {
        if (static_cast<int32_t>(def.size()) != _dim + 1) {
                throw SpaceError("defaultToTangent: size != dim + 1");
        }
        Pt res(_dim, 0.0L);
        for (int32_t i = 0; i < _dim; i++) {
                res[i] = def[i + 1];
        }
        return res;
}

Pt HyperbolicSpace::tangentToDefault(const Pt& at, const Pt& tangent) const {
        if (!isValid(at) || !isValid(tangent)) throw SpaceError("tangentToDefault: size != dim");
        ld x0 = std::sqrtl(1.0L + norm(at) * norm(at));
        ld dot = 0.0L;
        for (int32_t i = 0; i < _dim; i++) {
                dot += at[i] * tangent[i];
        }

        Pt res(_dim + 1, 0.0L);
        res[0] = dot / x0;
        for (int32_t i = 0; i < _dim; i++) {
                res[i + 1] = tangent[i];
        }
        return res;
}

ld HyperbolicSpace::dist(const Pt& a, const Pt& b) const {
        if (!isValid(a) || !isValid(b)) throw SpaceError("dist: size != dim");
        Pt A = lift(a);
        Pt B = lift(b);
        ld arg = -lorentzProd(A, B);
        arg = std::max(arg, 1.0L);
        return std::acoshl(arg);
}

ld HyperbolicSpace::norm(const Pt& vec) const {
        if (!isValid(vec)) throw SpaceError("norm: size != dim");
        ld summ = 0.0L;
        for (int32_t i = 0; i < _dim; i++) {
                summ += vec[i] * vec[i];
        }
        return std::sqrtl(summ);
}

Pt HyperbolicSpace::logMap(const Pt& from, const Pt& to) const {
        if (!isValid(from) || !isValid(to)) throw SpaceError("logMap: size != dim");
        const ld EPS = 1e-15L;
        Pt X = lift(from);
        Pt Y = lift(to);

        ld alpha = -lorentzProd(X, Y);
        alpha = std::max(alpha, 1.0L);
        ld d = std::acoshl(alpha);
        if (d < EPS) return Pt(_dim, 0.0L);

        Pt U(_dim + 1, 0.0L);
        for (int32_t i = 0; i <= _dim; i++) {
                U[i] = Y[i] - alpha * X[i];
        }

        ld uNormSq = lorentzProd(U, U);
        if (uNormSq < EPS) return Pt(_dim, 0.0L);
        ld uNorm = std::sqrtl(uNormSq);
        ld coeff = d / uNorm;
        for (int32_t i = 0; i <= _dim; i++) {
                U[i] *= coeff;
        }
        return defaultToTangent(U);
}

Pt HyperbolicSpace::expMap(const Pt& from, const Pt& tangent) const {
        if (!isValid(from) || !isValid(tangent)) throw SpaceError("expMap: size != dim");
        const ld EPS = 1e-15L;
        Pt X = lift(from);
        Pt V = tangentToDefault(from, tangent);

        ld vNormSq = lorentzProd(V, V);
        vNormSq = std::max(vNormSq, 0.0L);
        ld vNorm = std::sqrtl(vNormSq);
        if (vNorm < EPS) {
                return from;
        }

        ld c = std::coshl(vNorm);
        ld s = std::sinhl(vNorm) / vNorm;

        Pt Z(_dim + 1, 0.0L);
        for (int32_t i = 0; i <= _dim; i++) {
                Z[i] = c * X[i] + s * V[i];
        }
        return defaultToTangent(Z);
}

ld HyperbolicSpace::tangentNorm(const Pt& at, const Pt& tangent) const {
        Pt V = tangentToDefault(at, tangent);
        ld resSq = lorentzProd(V, V);
        resSq = std::max(resSq, 0.0L);
        return std::sqrtl(resSq);
}

Pt HyperbolicSpace::normalizePoint(const Pt& p, const std::vector<int32_t>& figSize) const {
        if (!isValid(p)) throw SpaceError("normalizePoint: size != dim");
        if (static_cast<int32_t>(figSize.size()) != _dim) {
                throw SpaceError("normalizePoint: figSize size != dim");
        }
        Pt res = p;
        for (int32_t i = 0; i < _dim; i++) {
                ld bound = (ld)figSize[i] / 2.0L;
                res[i] = std::min(bound, std::max(-bound, res[i]));
        }
        return res;
}

ld HyperbolicSpace::volume(const std::vector<int32_t>& figSize) const {
        if (static_cast<int32_t>(figSize.size()) != _dim) {
                throw SpaceError("volume: figSize size != dim");
        }

        Pt origin(_dim, 0.0L);
        std::vector<Pt> edges(_dim, Pt(_dim, 0.0L));
        for (int32_t i = 0; i < _dim; i++) {
                edges[i][i] = (ld)figSize[i];
        }

        std::vector<std::vector<ld>> gram(_dim, std::vector<ld>(_dim, 0.0L));
        for (int32_t i = 0; i < _dim; i++) {
                for (int32_t j = 0; j < _dim; j++) {
                        ld ni = tangentNorm(origin, edges[i]);
                        ld nj = tangentNorm(origin, edges[j]);
                        ld nij = tangentNorm(origin, edges[i] + edges[j]);
                        gram[i][j] = (nij * nij - ni * ni - nj * nj) / 2.0L;
                }
        }
        ld det = Space::determinantBareiss(gram);
        det = std::max(det, 0.0L);
        return std::sqrtl(det);
}

bool HyperbolicSpace::areGeodesicSegmentsCrossing(
                const Pt& a,
                const Pt& b,
                const Pt& c,
                const Pt& d) const {
        if (_dim != 2) {
                throw SpaceError("areGeodesicSegmentsCrossing: hyperbolic dimension != 2");
        }
        if (!isValid(a) || !isValid(b) || !isValid(c) || !isValid(d)) {
                throw SpaceError("areGeodesicSegmentsCrossing: size != dim");
        }
        auto toKlein = [&](const Pt& p) -> Pt {
                ld x0 = std::sqrtl(1.0L + p[0] * p[0] + p[1] * p[1]);
                return p / x0;
        };
        return areEuclideanSegmentsCrossing2D(
                toKlein(a), toKlein(b), toKlein(c), toKlein(d));
}

bool HyperbolicSpace::isValid(const Pt& c) const {
        if (static_cast<int32_t>(c.size()) != _dim) return false;
        for (int32_t i = 0; i < _dim; i++) {
                if (!std::isfinite(c[i])) return false;
        }
        return true;
}

} // namespace gd
