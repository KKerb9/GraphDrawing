#include "io/EmbeddingWriter.h"

#include <fstream>

namespace gd {

namespace {

void ensureOutputDirExists(const std::string& path) {
        (void)path;
}

} // namespace

void writeEmbeddingJson(const std::string& outputPath,
        const std::string& graphName,
        const std::string& algoName,
        const std::string& spaceName,
        const std::string& initialPlacementName,
        const Projection& proj,
        const Embedding& emb,
        const Metrics& metrics) {
        ensureOutputDirExists(outputPath);

        std::ofstream out(outputPath);
	if (!out) {
		throw EmbeddingWriterError("Failed to open output file: " + outputPath);
	}

        out << "{\n";
        out << "  \"graph_name\": \"" << graphName << "\",\n";
        out << "  \"algo\": \"" << algoName << "\",\n";
        out << "  \"space\": \"" << spaceName << "\",\n";
        out << "  \"initial_placement\": \"" << initialPlacementName << "\",\n";
        out << "  \"projection\": \"" << proj.name() << "\",\n";
        out << "  \"nodes\": [\n";

        const auto& coords = emb.all();
        for (std::int32_t v = 0; v < emb.size(); v++) {
                const auto& c = coords[v];
                const auto p = proj.project2D(c);

                out << "    {\"id\": " << v << ", \"x\": " << p.x << ", \"y\": " << p.y << "}";
                if (v + 1 < emb.size()) {
                        out << ',';
                }
                out << '\n';
        }

        out << "  ],\n";
        out << "  \"metrics\": {\n";
        out << "    \"stress\": " << metrics.stress << ",\n";
        out << "    \"edgeLengthVariance\": " << metrics.edgeLengthVariance << '\n';
        out << "  }\n";
        out << "}\n";
}

} // namespace gd

