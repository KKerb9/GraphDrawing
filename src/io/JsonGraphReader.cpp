#include "io/JsonGraphReader.h"

#include <fstream>
#include <stdexcept>

namespace gd {

JsonGraphReader::JsonGraphReader(const std::string& datasetPath) : _datasetPath(datasetPath) {}

Graph JsonGraphReader::readGraphByName(const std::string& graphName) const {
        (void)graphName;

        // TODO Реализовать разбор JSON из _datasetPath
	throw JsonGraphReaderError("readGraphByName is not implemented yet");
}

} // namespace gd

