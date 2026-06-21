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
	std::string borderPolicyName;
	std::string initialPlacementName;
	std::string projectionName;
	std::string datasetPath;
	std::string outputPath;
	int32_t dimension = 2;
	int32_t finalDimension = 2;
	uint32_t seed = 0;
	std::vector<int32_t> figSize;

	bool isValid() const;
};

Config parseArgs(int argc, char** argv);

struct FaRInteractiveParams {
	int32_t iters;
	long double c;
};

FaRInteractiveParams readFaRInteractiveParams(std::istream& in, std::ostream& out);

struct BorderPolicyInteractiveParams {
	std::vector<long double> sideSizes;
};

BorderPolicyInteractiveParams readBorderPolicyInteractiveParams(
	const std::string& borderPolicyName,
	int32_t dim,
	std::istream& in,
	std::ostream& out);

} // namespace gd
