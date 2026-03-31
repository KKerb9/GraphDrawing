#pragma once

#include "Space.h"

namespace gd {

class EuclideanSpace : public Space {
public:
        explicit EuclideanSpace(int32_t dim);

        std::string name() const override;

        int32_t dimension() const noexcept override;

        double dist(const Coord& a, const Coord& b) const override;

        bool isValid(const Coord& c) const override;

private:
        int32_t _dim;
        std::string _name;
};

} // namespace gd
