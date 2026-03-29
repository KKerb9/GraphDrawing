#pragma once

#include <cstddef>
#include <memory>
#include <string>

#include "../core/Embedding.h"
#include "../core/Errors.h"

namespace gd {

class SpaceError : public GraphDrawingError {
public:
	using GraphDrawingError::GraphDrawingError;
};

class Space {
public:
        virtual ~Space() = default;

        virtual std::string name() const = 0;

        virtual std::int32_t dimension() const = 0;

        virtual double dist(const Coord& a, const Coord& b) const = 0;

        virtual bool isValid(const Coord& c) const = 0;
};

std::unique_ptr<Space> createSpace(const std::string& spaceName, std::int32_t dim);

} // namespace gd
