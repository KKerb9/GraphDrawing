#pragma once

#include <memory>
#include <string>

#include "../core/Embedding.h"
#include "../core/Errors.h"
#include "../spaces/Space.h"

namespace gd {

class LayoutError : public GraphDrawingError {
public:
	using GraphDrawingError::GraphDrawingError;
};

class LayoutAlgorithm {
public:
        explicit LayoutAlgorithm(const std::string& name);

        virtual ~LayoutAlgorithm() = default;

        std::string name() const;

        virtual void computeLayout(Embedding& emb, const Space& space) const = 0;

private:
        std::string _name;
};

using LayoutAlgorithmPtr = std::unique_ptr<LayoutAlgorithm>;

LayoutAlgorithmPtr createLayoutAlgorithm(const std::string& name);

} // namespace gd

