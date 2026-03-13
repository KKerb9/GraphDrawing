#pragma once

#include <cstddef>
#include <memory>
#include <string>

#include "core/Embedding.h"
#include "core/Errors.h"

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

        virtual bool isValid(const Coord& c) const;
};

class EuclideanSpace : public Space {
public:
        explicit EuclideanSpace(std::int32_t dim);

        std::string name() const override;

        std::int32_t dimension() const noexcept override;

        double dist(const Coord& a, const Coord& b) const override;

private:
        std::int32_t _dim;
        std::string _name;
};

std::unique_ptr<Space> createSpace(const std::string& spaceName, std::int32_t dim);

} // namespace gd

