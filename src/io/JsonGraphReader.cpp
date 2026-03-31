#include "JsonGraphReader.h"

#include <algorithm>
#include <cstdint>
#include <fstream>
#include <limits>
#include <string>

#include <nlohmann/json.hpp>

namespace gd {

namespace {

std::string readFileToString(const std::string& path) {
	std::ifstream in(path, std::ios::binary);
	if (!in) {
		throw JsonGraphReaderError("cannot open dataset file: " + path);
	}
	in.seekg(0, std::ios::end);
	const std::streamoff len = in.tellg();
	if (len < 0) {
		throw JsonGraphReaderError("cannot read dataset file size: " + path);
	}
	in.seekg(0, std::ios::beg);
	std::string buf(static_cast<std::size_t>(len), '\0');
	if (len > 0) {
		in.read(&buf[0], len);
		if (!in) {
			throw JsonGraphReaderError("cannot read dataset file: " + path);
		}
	}
	if (buf.size() >= 3 && static_cast<unsigned char>(buf[0]) == 0xEF
	    && static_cast<unsigned char>(buf[1]) == 0xBB
	    && static_cast<unsigned char>(buf[2]) == 0xBF) {
		buf.erase(0, 3);
	}
	return buf;
}

std::int64_t jsonIntegerValue(const nlohmann::json& j) {
	if (j.is_number_integer()) {
		return j.get<std::int64_t>();
	}
	if (j.is_number_unsigned()) {
		const std::uint64_t u = j.get<std::uint64_t>();
		constexpr std::uint64_t max64 =
			static_cast<std::uint64_t>(std::numeric_limits<std::int64_t>::max());
		if (u > max64) {
			throw JsonGraphReaderError(
				"readGraphByName: integer out of signed 64-bit range");
		}
		return static_cast<std::int64_t>(u);
	}
	throw JsonGraphReaderError(
		"readGraphByName: expected JSON integer vertex id");
}

void ensureInt32VertexId(std::int64_t id, const char* context) {
	if (id < 0) {
		throw JsonGraphReaderError(std::string(context) + ": negative vertex id");
	}
	if (id > static_cast<std::int64_t>(
		    std::numeric_limits<int32_t>::max())) {
		throw JsonGraphReaderError(std::string(context)
					   + ": vertex id too large for int32");
	}
}

std::pair<int32_t, int32_t> parseEdgePair(const nlohmann::json& edge) {
	if (!edge.is_array() || edge.size() != 2) {
		throw JsonGraphReaderError(
			"readGraphByName: each edge must be a pair [u, v]");
	}
	const std::int64_t u0 = jsonIntegerValue(edge[0]);
	const std::int64_t v0 = jsonIntegerValue(edge[1]);
	ensureInt32VertexId(u0, "readGraphByName: edge");
	ensureInt32VertexId(v0, "readGraphByName: edge");
	return {static_cast<int32_t>(u0), static_cast<int32_t>(v0)};
}

int32_t vertexCountFromEntry(const nlohmann::json& entry) {
	std::int64_t maxId = -1;

	if (entry.contains("nodes")) {
		if (!entry["nodes"].is_array()) {
			throw JsonGraphReaderError(
				"readGraphByName: \"nodes\" must be a JSON array");
		}
		for (const auto& node : entry["nodes"]) {
			const std::int64_t id = jsonIntegerValue(node);
			ensureInt32VertexId(id, "readGraphByName: \"nodes\"");
			maxId = std::max(maxId, id);
		}
	}

	for (const auto& edge : entry.at("edges")) {
		const auto pr = parseEdgePair(edge);
		maxId = std::max(maxId, static_cast<std::int64_t>(pr.first));
		maxId = std::max(maxId, static_cast<std::int64_t>(pr.second));
	}

	if (maxId < 0) {
		return 0;
	}
	if (maxId == static_cast<std::int64_t>(
		    std::numeric_limits<int32_t>::max())) {
		throw JsonGraphReaderError(
			"readGraphByName: vertex id range overflow (max int32)");
	}
	return static_cast<int32_t>(maxId + 1);
}

} // namespace

JsonGraphReader::JsonGraphReader(const std::string& datasetPath)
	: _datasetPath(datasetPath) {}

Graph JsonGraphReader::readGraphByName(const std::string& graphName) const {
	const std::string buf = readFileToString(_datasetPath);

	nlohmann::json root;
	try {
		root = nlohmann::json::parse(buf);
	} catch (const nlohmann::json::parse_error& e) {
		throw JsonGraphReaderError(std::string("JSON parse error: ") + e.what());
	}

	if (!root.is_array()) {
		throw JsonGraphReaderError("dataset root must be a JSON array");
	}

	const nlohmann::json* entry = nullptr;
	for (const auto& item : root) {
		if (!item.is_object()) {
			continue;
		}
		auto it = item.find("name");
		if (it != item.end() && it->is_string()
		    && it->get<std::string>() == graphName) {
			entry = &item;
			break;
		}
	}

	if (entry == nullptr) {
		throw JsonGraphReaderError("graph not found in dataset: " + graphName);
	}

	if (!(*entry).contains("edges") || !(*entry)["edges"].is_array()) {
		throw JsonGraphReaderError(
			"readGraphByName: graph entry must contain \"edges\" array");
	}

	int32_t n = vertexCountFromEntry(*entry);
	Graph g(n);

	for (const auto& edge : (*entry)["edges"]) {
		auto pr = parseEdgePair(edge);
		int32_t u = pr.first;
		int32_t v = pr.second;
		if (std::max(u, v) >= n) {
			throw JsonGraphReaderError(
				"readGraphByName: edge endpoint out of range for graph");
		}
		g.addEdge(u, v);
	}

	return g;
}

} // namespace gd
