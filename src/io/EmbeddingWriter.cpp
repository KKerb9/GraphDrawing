#include "EmbeddingWriter.h"

#include <filesystem>
#include <fstream>
#include <iomanip>

namespace gd {

void checkOutDir(const std::string& path) {
	std::filesystem::path dir = std::filesystem::path(path).parent_path();
	if (dir.empty()) {
		return;
	}
	if (!std::filesystem::exists(dir)) {
		std::filesystem::create_directories(dir);
	}
}

void writeEmbeddingJson(const Config& cfg, const Embedding& res, const Metrics& metrics) {
	checkOutDir(cfg.outputPath);

	std::ofstream out(cfg.outputPath);
	if (!out) {
		throw EmbeddingWriterError("Failed to open output file: " + cfg.outputPath);
	}

	out << "{\n";
	out << "  \"graph_name\": \"" << cfg.graphName << "\",\n";
	out << "  \"algo\": \"" << cfg.algoName << "\",\n";
	out << "  \"space\": \"" << cfg.spaceName << "\",\n";
	out << "  \"initial_placement\": \"" << cfg.initialPlacementName << "\",\n";
	out << "  \"projection\": \"" << cfg.projectionName << "\",\n";
        out << "  \"dimension\": " << res.dimension() << ",\n";
	out << "  \"nodes\": [\n";

	{
		const auto& coords = res.getCoords();
		int32_t n = res.size();
		for (int32_t i = 0; i < n; i++) {
			const auto& c = coords[i];
			out << "    {\"id\": " << i << ", \"x\": " << std::fixed << std::setprecision(6) << c[0] << ", \"y\": " << c[1];
			if (res.dimension() == 3) {
				out << ", \"z\": " << std::fixed << std::setprecision(6) << c[2];
			}
			out << "}";
                        out.unsetf(std::ios::floatfield);
			if (i + 1 < n) {
				out << ",";
			}
			out << "\n";
		}
	}

	out << "  ],\n";
	out << "  \"edges\": [\n";
	{
		const auto& edges = res.getGraph().edges();
		int32_t n = static_cast<int32_t>(edges.size());
		for (int32_t i = 0; i < n; i++) {
			out << "    [" << edges[i].first << ", " << edges[i].second << "]";
			if (i + 1 < n) {
				out << ",";
			}
			out << "\n";
		}
	}
	out << "  ],\n";
        out << std::fixed << std::setprecision(6);
	out << "  \"metrics\": {\n";
	out << "    \"volume\": "  << metrics.volume << ",\n";
	out << "    \"minVertexDist\": " << metrics.minVertexDist << ",\n";
	out << "    \"maxVertexDist\": " << metrics.maxVertexDist << ",\n";
	out << "    \"avgVertexDist\": " << metrics.avgVertexDist << ",\n";
        out.unsetf(std::ios::floatfield);
	out << "    \"edgeCrossings\": " << metrics.edgeCrossings << ",\n";
        out << std::fixed << std::setprecision(6);
	out << "    \"minAngle\": " << metrics.minAngle << ",\n";
	out << "    \"maxAngle\": " << metrics.maxAngle << ",\n";
	out << "    \"density\": " << metrics.density << "\n";
	out << "  }\n";
	out << "}\n";
}

} // namespace gd

