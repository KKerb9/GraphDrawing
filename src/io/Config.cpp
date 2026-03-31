#include "Config.h"

#include <cstdlib>
#include <iostream>
#include <string>

namespace gd {

namespace {

const std::vector<std::string> ALGO_NAMES = {"random"};
const std::vector<std::string> SPACE_NAMES = {"euclidean", "hyperbolic", "spherical"};
const std::vector<std::string> INITIAL_PLACEMENT_NAMES = {"zero"};
const std::vector<std::string> PROJECTION_NAMES = {"euclidean", "hyperbolic", "spherical"};

bool contains(const std::vector<std::string>& list, const std::string& s) {
	for (const auto& x : list) {
		if (x == s) return true;
	}
	return false;
}

void printHelp() {
	std::cerr << "Options:\n";
	std::cerr << "\t--graph <name>\n";
	std::cerr << "\t--algo <name> (";
	for (const auto& n : ALGO_NAMES) std::cerr << n << ", ";
	std::cerr << ")\n";
	std::cerr << "\t--space <name> (";
	for (const auto& n : SPACE_NAMES) std::cerr << n << ", ";
	std::cerr << ")\n";
	std::cerr << "\t--initial-placement <name> (";
	for (const auto& n : INITIAL_PLACEMENT_NAMES) std::cerr << n << ", ";
	std::cerr << ")\n";
	std::cerr << "\t--projection <name> (";
	for (const auto& n : PROJECTION_NAMES) std::cerr << n << ", ";
	std::cerr << ")\n";
	std::cerr << "\t--dataset <path>\n";
	std::cerr << "\t--output <path>\n";
	std::cerr << "\t--help\n";
}

} // namespace

bool Config::isValid() const {
	return !graphName.empty()
		&& contains(ALGO_NAMES, algoName)
		&& contains(SPACE_NAMES, spaceName)
		&& contains(INITIAL_PLACEMENT_NAMES, initialPlacementName)
		&& contains(PROJECTION_NAMES, projectionName);
}

Config parseArgs(int argc, char** argv) {
	Config cfg;

	for (int i = 1; i < argc; i++) {
		const std::string arg = argv[i];
		if (arg == "--help") {
			printHelp();
			std::exit(0);
		}
		if (arg == "--graph") {
			if (i + 1 >= argc) throw ConfigError("--graph requires a value");
			cfg.graphName = argv[++i];
		} else if (arg == "--algo") {
			if (i + 1 >= argc) throw ConfigError("--algo requires a value");
			cfg.algoName = argv[++i];
		} else if (arg == "--space") {
			if (i + 1 >= argc) throw ConfigError("--space requires a value");
			cfg.spaceName = argv[++i];
		} else if (arg == "--initial-placement") {
			if (i + 1 >= argc) throw ConfigError("--initial-placement requires a value");
			cfg.initialPlacementName = argv[++i];
		} else if (arg == "--projection") {
			if (i + 1 >= argc) throw ConfigError("--projection requires a value");
			cfg.projectionName = argv[++i];
		} else if (arg == "--dataset") {
			if (i + 1 >= argc) throw ConfigError("--dataset requires a value");
			cfg.datasetPath = argv[++i];
		} else if (arg == "--output") {
			if (i + 1 >= argc) throw ConfigError("--output requires a value");
			cfg.outputPath = argv[++i];
		} else {
			throw ConfigError("Unknown argument: " + arg);
		}
	}

	if (cfg.datasetPath.empty()) {
		cfg.datasetPath = "samples/dataset.json";
	}
	if (cfg.outputPath.empty()) {
		cfg.outputPath = "out/" + cfg.graphName + "_" + cfg.algoName + ".json";
	}

	if (!cfg.isValid()) {
		throw ConfigError("Invalid configuration: missing or invalid options");
	}

	return cfg;
}

} // namespace gd
