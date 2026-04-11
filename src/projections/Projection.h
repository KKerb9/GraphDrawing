#pragma once

#include <memory>
#include <string>

#include "../core/Embedding.h"
#include "../core/Errors.h"
#include "../spaces/Space.h"

namespace gd {

class ProjectionError : public GraphDrawingError {
public:
	using GraphDrawingError::GraphDrawingError;
};

class Projection {
public:
	virtual ~Projection() = default;

	virtual std::string name() const = 0;

	virtual Embedding project(const Embedding& emb, const Space& space, int32_t finalDim) const = 0;
};

class IdentityProjection : public Projection {
public:
	explicit IdentityProjection();

	std::string name() const override;

	Embedding project(const Embedding& emb, const Space& space, int32_t finalDim) const override;

private:
	std::string _name;
};

class OrthogonalProjection : public Projection {
public:
	explicit OrthogonalProjection();

	std::string name() const override;

	Embedding project(const Embedding& emb, const Space& space, int32_t finalDim) const override;

private:
	std::string _name;
};

using ProjectionPtr = std::unique_ptr<Projection>;

ProjectionPtr createProjection(const std::string& projName);

} // namespace gd

