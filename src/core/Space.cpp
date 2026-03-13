#include "core/Space.h"

#include <cmath>

namespace gd {

bool Space::isValid(const Coord& c) const {
        return static_cast<std::int32_t>(c.size()) == _dim;
        // throw SpaceError("coordinate dimension mismatch");
}

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

std::unique_ptr<Space> createSpace(const std::string& spaceName, std::int32_t dim) {
	if (spaceName == "euclidean") {
		return std::make_unique<EuclideanSpace>(dim);
	}
	throw SpaceError("createSpace: unknown space name: " + spaceName);
}

} // namespace gd

