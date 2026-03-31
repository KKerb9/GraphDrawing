#pragma once

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

	bool isValid() const;
};

Config parseArgs(int argc, char** argv);

} // namespace gd
