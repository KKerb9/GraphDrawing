#pragma once

#include "Space.h"

namespace gd {

class PoincareSpace : public Space {
public:
	PoincareSpace(int32_t dim, ld radius);

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

	ld radius() const noexcept;

private:
	ld euclideanNorm(const Pt& p) const;  // аналог евклидовой нормы но без проверки точек
	Pt toSmallDisk(const Pt& p) const;  // переводит в диск с единичным радиусом
	Pt toDefDisk(const Pt& p) const;  // переводит в диск с нормальным радиусом
	Pt mobiusAdd(const Pt& a, const Pt& b) const;  // сложение векторов

	int32_t _dim;
	ld _radius;
	std::string _name;
};

} // namespace gd
