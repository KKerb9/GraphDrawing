#include "EmbeddingWriter.h"

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

        {
                auto tmp = emb.getCoords();
                int n = tmp.size();
                for (std::int32_t i = 0; i < n - 1; i++) {
                        const auto& c = tmp[i];
                        auto p = Vec2{0.0, 0.0};
                        if (proj.name() == "identity") {
                                p = Vec2{c[0], c[1]};
                        } else if (proj.name() == "euclidean") {
                                p = proj.project2D(c);
                        } else {
                                throw EmbeddingWriterError("Unknown projection name: " + proj.name());
                        }

                        out << "    {\"id\": " << i << ", \"x\": " << p.x << ", \"y\": " << p.y << "},\n";
                }
                const auto& c = tmp[n - 1];
                auto p = Vec2{0.0, 0.0};
                if (proj.name() == "identity") {
                        p = Vec2{c[0], c[1]};
                } else if (proj.name() == "euclidean") {
                        p = proj.project2D(c);
                } else {
                        throw EmbeddingWriterError("Unknown projection name: " + proj.name());
                }
                out << "    {\"id\": " << n - 1 << ", \"x\": " << p.x << ", \"y\": " << p.y << "}\n";
        }

        out << "  ],\n";
        out << "  \"edges\": [\n";
        {
                auto tmp = emb.getGraph().edges();
                int n = tmp.size();
                for (std::int32_t i = 0; i < n - 1; i++) {
                        out << "    [" << tmp[i].first << ", " << tmp[i].second << "],\n";
                }
                out << "    [" << tmp[n - 1].first << ", " << tmp[n - 1].second << "]\n";
        }
        out << "  ],\n";
        out << "  \"metrics\": {\n";
        out << "    \"stress\": " << metrics.stress << ",\n";
        out << "    \"edgeLengthVariance\": " << metrics.edgeLengthVariance << '\n';
        out << "  }\n";
        out << "}\n";
}

} // namespace gd

