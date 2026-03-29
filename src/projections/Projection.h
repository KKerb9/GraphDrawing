#pragma once

#include <memory>
#include <string>

#include "../core/Embedding.h"
#include "../core/Errors.h"

namespace gd {

class ProjectionError : public GraphDrawingError {
public:
	using GraphDrawingError::GraphDrawingError;
};

class Projection {
public:
	virtual ~Projection() = default;

	virtual std::string name() const = 0;

	virtual Vec2 project2D(const Coord& c) const = 0;

	virtual Vec3 project3D(const Coord& c) const = 0;
};

class IdentityProjection : public Projection {
public:
        explicit IdentityProjection(std::int32_t dim);

        std::string name() const override;

	Vec2 project2D(const Coord& c) const override;

	Vec3 project3D(const Coord& c) const override;

private:
        std::int32_t _dim;
        std::string _name;
};

std::unique_ptr<Projection> createProjection(const std::string& spaceName, std::int32_t dim);

} // namespace gd

