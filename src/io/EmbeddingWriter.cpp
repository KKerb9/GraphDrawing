#include "EmbeddingWriter.h"

#include <fstream>

namespace gd {

namespace {

void ensureOutputDirExists(const std::string& path) {
        (void)path;
}

} // namespace

void writeEmbeddingJson(
	const Config& cfg,
	const Embedding& result,
	const Metrics& metrics) {
	ensureOutputDirExists(cfg.outputPath);

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
        out << "  \"dimension\": " << result.dimension() << ",\n";
	out << "  \"nodes\": [\n";

	{
		const auto& coords = result.getCoords();
		int32_t n = result.size();
		for (int32_t i = 0; i < n; i++) {
			const auto& c = coords[i];
			out << "    {\"id\": " << i << ", \"x\": " << c[0] << ", \"y\": " << c[1];
			if (result.dimension() == 3) {
				out << ", \"z\": " << c[2];
			}
			out << "}";
			if (i + 1 < n) {
				out << ",";
			}
			out << "\n";
		}
	}

	out << "  ],\n";
	out << "  \"edges\": [\n";
	{
		const auto& edges = result.getGraph().edges();
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
	out << "  \"metrics\": {\n";
	out << "    \"volume\": " << metrics.volume << ",\n";
	out << "    \"minVertexDist\": " << metrics.minVertexDist << ",\n";
	out << "    \"maxVertexDist\": " << metrics.maxVertexDist << ",\n";
	out << "    \"avgVertexDist\": " << metrics.avgVertexDist << ",\n";
	out << "    \"edgeCrossings\": " << metrics.edgeCrossings << ",\n";
	out << "    \"minAngle\": " << metrics.minAngle << ",\n";
	out << "    \"maxAngle\": " << metrics.maxAngle << ",\n";
	out << "    \"density\": " << metrics.density << "\n";
	out << "  }\n";
	out << "}\n";
}

} // namespace gd

