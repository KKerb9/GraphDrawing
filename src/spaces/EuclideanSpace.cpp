#include "EuclideanSpace.h"

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
        // if (!EuclideanSpace::isValid(a) || !EuclideanSpace::isValid(b)) {
        //         throw SpaceError("EuclideanSpace::distance: dimension mismatch");
        // }
        ld sum = 0.0;
        for (int32_t i = 0; i < _dim; i++) {
            ld d = a[i] - b[i];
            sum += d * d;
        }
        return std::sqrt(sum);
}

bool EuclideanSpace::isValid(const Pt& c) const {
        return static_cast<int32_t>(c.size()) == _dim;
        // throw SpaceError("coordinate dimension mismatch");
}

ld EuclideanSpace::norm(const Pt& vec) const {
        ld sum = 0.0;
        for (const ld x : vec) {
                sum += x * x;
        }
        return std::sqrtl(sum);
}

ll EuclideanSpace::area(const std::vector<int32_t>& size) const {
        ll res = 1;
        for (const auto x : size) {
                res *= x;
        }
        return res;
}

} // namespace gd
