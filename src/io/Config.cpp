#include "Config.h"

#include <cstdlib>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>

namespace gd {

const std::vector<std::string> ALGO_NAMES = {"random", "far"};
const std::vector<std::string> SPACE_NAMES = {"euclidean", "hyperbolic", "spherical"};
const std::vector<std::string> INITIAL_PLACEMENT_NAMES = {"zero", "random"};
const std::vector<std::string> PROJECTION_NAMES = {"euclidean", "hyperbolic", "spherical"};

bool contains(const std::vector<std::string>& list, const std::string& s) {
	for (const auto& x : list) {
		if (x == s) return true;
	}
	return false;
}

void printHelp() {
	std::cerr << "Options (defaults allow running with no arguments):\n";
	std::cerr << "\t--graph <name> (default: SmallGraph)\n";
	std::cerr << "\t--algo <name> (";
	for (const auto& n : ALGO_NAMES) std::cerr << n << ", ";
	std::cerr << "; default: random; far = Fruchterman-Reingold, prompts for ITERS/C on stdin)\n";
	std::cerr << "\t--space <name> (";
	for (const auto& n : SPACE_NAMES) std::cerr << n << ", ";
	std::cerr << "; default: euclidean)\n";
	std::cerr << "\t--initial-placement <name> (";
	for (const auto& n : INITIAL_PLACEMENT_NAMES) std::cerr << n << ", ";
	std::cerr << "; default: zero)\n";
	std::cerr << "\t--projection <name> (";
	for (const auto& n : PROJECTION_NAMES) std::cerr << n << ", ";
	std::cerr << "; default: euclidean)\n";
	std::cerr << "\t--dim <2|3> (default: 2); put before --FS if dimension is not 2\n";
	std::cerr << "\t--FS <n1,n2,...> figure size: exactly <dimension> integers (default: 100 each)\n";
	std::cerr << "\t--dataset <path> (default: samples/dataset.json)\n";
	std::cerr << "\t--output <path> (default: out/<graph>_<algo>.json)\n";
	std::cerr << "\t--help\n";
	std::cerr << "--algo far: one line after start: --C x --I n or empty = defaults (FigSize via --FS)\n";
}

constexpr int32_t kDefaultFaRIters = 100;
constexpr long double kDefaultFaRC = 1.0;
constexpr int32_t kDefaultFaRFigSide = 100;

std::vector<int32_t> parseFigSize(std::string s, int32_t dim) {
	for (char& ch : s) {
		if (ch == ',') ch = ' ';
	}
	std::istringstream ss(s);
	std::vector<int32_t> out;
	int32_t x = 0;
	for (int i = 0; i < dim; i++) {
		if (!(ss >> x)) {
			throw ConfigError("--FS: need " + std::to_string(dim) + " numbers");
		}
		out.push_back(x);
	}
	std::string extra;
	if (ss >> extra) {
		throw ConfigError("--FS: too many numbers");
	}
	return out;
}

bool Config::isValid() const {
	// std::cerr << graphName << ' ' << algoName << ' ' << spaceName << ' ' << initialPlacementName << ' ' << projectionName << '\n';
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
		} else if (arg == "--dim") {
			if (i + 1 >= argc) throw ConfigError("--dim requires a value");
			cfg.dimension = std::stoi(argv[++i]);
		} else if (arg == "--FS") {
			if (i + 1 >= argc) throw ConfigError("--FS requires a value");
			cfg.figSize = parseFigSize(argv[++i], cfg.dimension);
		} else {
			throw ConfigError("Unknown argument: " + arg);
		}
	}

	if (cfg.graphName.empty()) {
		cfg.graphName = "SmallGraph";
	}
	if (cfg.algoName.empty()) {
		cfg.algoName = "random";
	}
	if (cfg.spaceName.empty()) {
		cfg.spaceName = "euclidean";
	}
	if (cfg.initialPlacementName.empty()) {
		cfg.initialPlacementName = "zero";
	}
	if (cfg.projectionName.empty()) {
		cfg.projectionName = "euclidean";
	}
	if (cfg.datasetPath.empty()) {
		cfg.datasetPath = "samples/dataset.json";
	}
	if (cfg.outputPath.empty()) {
		cfg.outputPath = "out/" + cfg.graphName + "_" + cfg.algoName + ".json";
	}

	if (cfg.figSize.empty()) {
		cfg.figSize = std::vector<int32_t>(cfg.dimension, kDefaultFaRFigSide);
	} else if (static_cast<int>(cfg.figSize.size()) != cfg.dimension) {
		throw ConfigError("figSize length != dimension");
	}

	if (!cfg.isValid()) {
		throw ConfigError("Invalid configuration: missing or invalid options");
	}

	return cfg;
}

FaRInteractiveParams readFaRInteractiveParams(std::istream& in, std::ostream& out) {
	out << "FaR (--C --I), empty = defaults: " << std::flush;
	std::string line;
	if (!std::getline(in, line)) {
		throw ConfigError("FaR: EOF");
	}

	int32_t iters = kDefaultFaRIters;
	long double c = kDefaultFaRC;

	if (line.empty()) {
		return FaRInteractiveParams{iters, c};
	}

	std::istringstream ls(line);
	std::string flag;
	while (ls >> flag) {
		if (flag != "--C" && flag != "--I") {
			throw ConfigError("FaR: expected --C / --I, got: " + flag);
		}
		std::string val;
		if (!(ls >> val)) {
			throw ConfigError("FaR: value after " + flag);
		}
		try {
			if (flag == "--C") {
				c = std::stold(val);
			} else {
				iters = static_cast<int32_t>(std::stol(val));
			}
		} catch (const std::exception&) {
			throw ConfigError("FaR: bad number (" + flag + ")");
		}
	}
	return FaRInteractiveParams{iters, c};
}

} // namespace gd
