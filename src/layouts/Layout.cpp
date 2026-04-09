#include "Layout.h"
#include "FruchtermanAndReingold.h"
#include "RandomLayout.h"

#include "../io/Config.h"

namespace gd {

LayoutAlgorithm::LayoutAlgorithm(std::string name)
	: _name(std::move(name)) {}

std::string LayoutAlgorithm::name() const {
	return _name;
}

LayoutAlgorithmPtr createLayoutAlgorithm(const std::string& name) {
	if (name == "random") {
		return std::make_unique<RandomLayout>();
	} else if (name == "far") {
		FaRInteractiveParams p = readFaRInteractiveParams(std::cin, std::cerr);
		return std::make_unique<FruchtermanAndReingoldLayout>(p.iters, p.c);
	} else {
		throw LayoutError("Unknown layout algorithm: " + name);
	}
}

} // namespace gd
