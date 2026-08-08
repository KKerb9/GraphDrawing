/*
Lorentzian space
*/

#pragma once

#include "Space.h"

namespace gd {

class HyperbolicSpace : public Space {
public:
        explicit HyperbolicSpace(int32_t dim);

        std::string name() const override;

        int32_t dimension() const noexcept override;

        ld dist(const Pt& a, const Pt& b) const override;

        ld norm(const Pt& vec) const override;

        Pt logMap(const Pt& from, const Pt& to) const override;

        Pt expMap(const Pt& from, const Pt& tangent) const override;

        ld tangentNorm(const Pt& at, const Pt& tangent) const override;

        Pt normalizePoint(const Pt& p, const std::vector<int32_t>& figSize) const override;

        ld volume(const std::vector<int32_t>& figSize) const override;

        bool areGeodesicSegmentsCrossing(const Pt& a, const Pt& b, const Pt& c, const Pt& d) const override;

        bool isValid(const Pt& c) const override;

        Pt lift(const Pt& x) const;

        Pt toKlein(const Pt& x) const;

        Pt tangentToDefault(const Pt& at, const Pt& tangent) const;
        Pt defaultToTangent(const Pt& def) const;

        ld lorentzProd(const Pt& a, const Pt& b) const;

private:
        int32_t _dim;
        std::string _name;
};

} // namespace gd
