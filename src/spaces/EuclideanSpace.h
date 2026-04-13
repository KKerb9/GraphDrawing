#pragma once

#include "Space.h"

namespace gd {

class EuclideanSpace : public Space {
public:
        explicit EuclideanSpace(int32_t dim);

        std::string name() const override;

        int32_t dimension() const noexcept override;

        ld dist(const Pt& a, const Pt& b) const override;

        ld norm(const Pt& vec) const override;

        ll area(const std::vector<int32_t>& size) const override;

        bool isValid(const Pt& c) const override;

private:
        int32_t _dim;
        std::string _name;
};

} // namespace gd
