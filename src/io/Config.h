#pragma once

#include <cstdint>
#include <iosfwd>
#include <string>
#include <vector>

#include "../core/Errors.h"

namespace gd {

class ConfigError : public GraphDrawingError {
public:
	using GraphDrawingError::GraphDrawingError;
};

struct Config {
	std::string graphName;
	std::string algoName;
	std::string spaceName;
	std::string initialPlacementName;
	std::string projectionName;
	std::string datasetPath;
	std::string outputPath;
	int32_t dimension = 2;
	std::vector<int32_t> figSize;

	bool isValid() const;
};

Config parseArgs(int argc, char** argv);

struct FaRInteractiveParams {
	int32_t iters;
	long double c;
};

FaRInteractiveParams readFaRInteractiveParams(std::istream& in, std::ostream& out);

} // namespace gd
