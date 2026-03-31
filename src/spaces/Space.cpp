#include "Space.h"

#include "EuclideanSpace.h"

namespace gd {

SpacePtr createSpace(const std::string& spaceName, int32_t dim) {
	if (spaceName == "euclidean") {
		return std::make_unique<EuclideanSpace>(dim);
	}
	throw SpaceError("createSpace: unknown space name: " + spaceName);
}

} // namespace gd
