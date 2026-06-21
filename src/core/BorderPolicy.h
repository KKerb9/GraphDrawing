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

struct BorderPolicyInteractiveParams;

class BorderPolicyError : public GraphDrawingError {
public:
	using GraphDrawingError::GraphDrawingError;
};

class BorderPolicy {
public:
	explicit BorderPolicy(std::string name);

	virtual ~BorderPolicy() = default;

	std::string name() const;

	virtual Pt force(const Space& space, const Pt& point) const = 0;

	virtual Pt normalizePoint(const Space& space, const Pt& point) const = 0;

	virtual Pt randomPoint(const Space& space) const = 0;

	virtual ld domainVolume(const Space& space) const = 0;

private:
	std::string _name;
};

// ничего не делает
class DefaultBorderPolicy : public BorderPolicy {
public:
	DefaultBorderPolicy(const BorderPolicyInteractiveParams& params, uint32_t seed);

	Pt force(const Space& space, const Pt& point) const override;

	Pt normalizePoint(const Space& space, const Pt& point) const override;

	Pt randomPoint(const Space& space) const override;

	ld domainVolume(const Space& space) const override;

private:
	mutable std::mt19937 _rng;
};

using BorderPolicyPtr = std::unique_ptr<BorderPolicy>;

// принимает seed, так как потом border отвечает за случайные точки внутри него
BorderPolicyPtr createBorderPolicy(const std::string& name, int32_t dim, uint32_t seed);

} // namespace gd
