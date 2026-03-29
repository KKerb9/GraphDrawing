#include "Space.h"

#include "EuclideanSpace.h"

namespace gd {

std::unique_ptr<Space> createSpace(const std::string& spaceName, std::int32_t dim) {
	if (spaceName == "euclidean") {
		return std::make_unique<EuclideanSpace>(dim);
	}
	throw SpaceError("createSpace: unknown space name: " + spaceName);
}

} // namespace gd
