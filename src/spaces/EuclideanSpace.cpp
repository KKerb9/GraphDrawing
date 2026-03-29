#include "EuclideanSpace.h"

#include <cmath>

namespace gd {

EuclideanSpace::EuclideanSpace(std::int32_t dim) : _dim(dim), _name("euclidean") {}

std::string EuclideanSpace::name() const {
        return _name;
}

std::int32_t EuclideanSpace::dimension() const noexcept {
        return _dim;
}

double EuclideanSpace::dist(const Coord& a, const Coord& b) const {
        // if (!EuclideanSpace::isValid(a) || !EuclideanSpace::isValid(b)) {
        //         throw SpaceError("EuclideanSpace::distance: dimension mismatch");
        // }
        double sum = 0.0;
        for (std::int32_t i = 0; i < _dim; i++) {
            double d = a[i] - b[i];
            sum += d * d;
        }
        return std::sqrt(sum);
}

bool EuclideanSpace::isValid(const Coord& c) const {
        return static_cast<std::int32_t>(c.size()) == _dim;
        // throw SpaceError("coordinate dimension mismatch");
}

} // namespace gd
