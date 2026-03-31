#pragma once

#include <string>

#include "../core/Errors.h"
#include "../core/Graph.h"

namespace gd {

class JsonGraphReaderError : public GraphDrawingError {
public:
	using GraphDrawingError::GraphDrawingError;
};

class JsonGraphReader {
public:
        explicit JsonGraphReader(const std::string& datasetPath);

        Graph readGraphByName(const std::string& graphName) const;

private:
        std::string _datasetPath;
};

} // namespace gd

