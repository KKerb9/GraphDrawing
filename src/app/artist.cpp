#include <cstdlib>
#include <iostream>
#include <memory>

#include "../core/Embedding.h"
#include "../core/Errors.h"
#include "../core/Graph.h"
#include "../core/InitialPlacement.h"
#include "../projections/Projection.h"
#include "../spaces/Space.h"
#include "../io/Config.h"
#include "../io/EmbeddingWriter.h"
#include "../io/JsonGraphReader.h"
#include "../layouts/Layout.h"
#include "../metrics/Metrics.h"

using namespace gd;

int main(int argc, char** argv) {
	try {
		Config cfg = parseArgs(argc, argv);
		
                JsonGraphReader reader(cfg.datasetPath);
                Graph graph = reader.readGraphByName(cfg.graphName);

                SpacePtr space = createSpace(cfg.spaceName, cfg.dimension);

                ProjectionPtr proj = createProjection(cfg.spaceName, space->dimension());

                Embedding emb(graph, cfg.dimension);

                InitialPlacementStrategyPtr init = createInitialPlacementStrategy(cfg.initialPlacementName);
                init->computeInitial(emb, *space, cfg.figSize);

                LayoutAlgorithmPtr algo = createLayoutAlgorithm(cfg.algoName);
                algo->computeLayout(emb, *space, cfg.figSize);

                MetricsCalculator mc;
                Metrics m = mc.compute(emb, *space);

                writeEmbeddingJson(
                        cfg.outputPath,
                        cfg.graphName,
                        cfg.algoName,
                        cfg.spaceName,
                        cfg.initialPlacementName,
                        *proj,
                        emb,
                        m
                );
                
		return 0;
	} catch (const GraphDrawingError& e) {
		std::cerr << "Error: " << e.what() << '\n';
		return 1;
	} catch (const std::exception& e) {
		std::cerr << "Error: " << e.what() << '\n';
		return 1;
	} catch (...) {
		std::cerr << "Unknown error\n";
		return 1;
	}
}
