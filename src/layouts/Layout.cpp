#include "Layout.h"
#include "RandomLayout.h"

namespace gd {

LayoutAlgorithm::LayoutAlgorithm(const std::string& name)
        : _name(name) {}

std::string LayoutAlgorithm::name() const {
        return _name;
}

LayoutAlgorithmPtr createLayoutAlgorithm(const std::string& name) {
        if (name == "random") {
                return std::make_unique<RandomLayout>();
        }

	throw LayoutError("Unknown layout algorithm: " + name);
}

} // namespace gd
