#pragma once

#include <stdexcept>
#include <string>

namespace gd {

class GraphDrawingError : public std::runtime_error {
public:
	explicit GraphDrawingError(const std::string& what) : std::runtime_error(what) {}
};

} // namespace gd
