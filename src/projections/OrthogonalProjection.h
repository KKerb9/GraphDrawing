#pragma once

#include "Projection.h"

namespace gd {

// берет первые finalDim координат
class OrthogonalProjection : public Projection {
public:
	explicit OrthogonalProjection();

	std::string name() const override;

	ProjectionResult project(
		const Embedding& emb,
		const Space& space,
		const std::vector<int32_t>& figSize,
		int32_t finalDim) const override;

private:
	std::string _name;
};

} // namespace gd
