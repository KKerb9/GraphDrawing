#include <cstdlib>
#include <iostream>
#include <memory>

#include "../core/Embedding.h"
#include "../core/Errors.h"
#include "../core/Graph.h"
#include "../core/InitialPlacement.h"
#include "../borderPolicy/BorderPolicy.h"
#include "../projections/Projection.h"
#include "../spaces/Space.h"
#include "../io/Config.h"
#include "../io/EmbeddingWriter.h"
#include "../io/JsonGraphReader.h"
#include "../layouts/Layout.h"
#include "../metrics/Metrics.h"

using namespace gd;

int main(int argc, char** argv) {
        std::cerr << "START\n";
	try {
		Config cfg = parseArgs(argc, argv);
		
                JsonGraphReader reader(cfg.datasetPath);
                Graph graph = reader.readGraphByName(cfg.graphName);

                SpacePtr space = createSpace(cfg.spaceName, cfg.dimension);
                BorderPolicyPtr borderPolicy = createBorderPolicy(
                        cfg.borderPolicyName,
                        cfg.dimension,
                        cfg.seed
                );

                ProjectionPtr proj = createProjection(cfg.projectionName, cfg.seed, cfg.cameraCandidates);

                Embedding emb(graph, cfg.dimension);

                InitialPlacementStrategyPtr init = createInitialPlacementStrategy(cfg.initialPlacementName);
                init->computeInitial(emb, *space, *borderPolicy);

                LayoutAlgorithmPtr algo = createLayoutAlgorithm(cfg.algoName);
                algo->computeLayout(emb, *space, *borderPolicy);

                ProjectionResult res = proj->project(
                        emb,
                        *space,
                        cfg.figSize,
                        cfg.finalDimension
                );

                Metrics metrics = computeMetrics(
                        res.embedding,
                        *res.space
                );

                writeEmbeddingJson(
                        cfg,
                        res.embedding,
                        *res.space,
                        metrics
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
