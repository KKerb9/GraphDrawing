#pragma once

#include <cstdint>
#include <memory>
#include <random>
#include <string>
#include <vector>

#include "Errors.h"
#include "template.h"
#include "../spaces/Space.h"

namespace gd {

class PolyBorderPolicy {
public:
	explicit PolyBorderPolicy(std::string name, int32_t dim, const std::vector<Pt>& figSize);

	virtual ~PolyBorderPolicy() = default;

	std::string name() const;

	virtual Pt force(const Space& space, const Pt& point) const = 0;

	virtual Pt normalizePoint(const Space& space, const Pt& point) const = 0;

	virtual Pt randomPoint(const Space& space) const = 0;

	virtual ld domainVolume(const Space& space) const = 0;

private:
	std::string _name;
        int32_t _dim;
        std::vector<Pt> _figSize;
};
} // namespace gd


/*
доделай PolyBorderPolicy по размеченному мной шаблону. figSize принимает размеры, причем у нас poly вокруг главной точки (0, ..., 0), а остальные стороны как бы пополам делятся центром. То есть если в figSize есть число 10, то получим в одну сторону -5 в другую 5.
Так же размерность poly тоже должна передаваться в аргументах.
Максимально соблюдай весь кодстайл проекта и аутентичноть. Перепроверяй себя.
*/