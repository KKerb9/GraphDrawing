#include "Projection.h"

namespace gd {

IdentityProjection::IdentityProjection(std::int32_t dim) : _dim(dim), _name("identity") {}

std::string IdentityProjection::name() const {
        return _name;
}

Vec2 IdentityProjection::project2D(const Coord& c) const {
        if (static_cast<std::int32_t>(c.size()) < 2) {
                throw ProjectionError("IdentityProjection::project2D: not enough coordinates");
        }
        return Vec2{c[0], c[1]};
}

Vec3 IdentityProjection::project3D(const Coord& c) const {
        if (c.size() < 3) {
                throw ProjectionError("IdentityProjection::project3D: not enough coordinates");
        }
        return Vec3{c[0], c[1], c[2]};
}

std::unique_ptr<Projection> createProjection(const std::string& spaceName, std::int32_t dim) {
        if (spaceName == "euclidean") {
                return std::make_unique<IdentityProjection>(dim);
        }
        throw ProjectionError("createProjection: unknown space name: " + spaceName);
}

} // namespace gd

