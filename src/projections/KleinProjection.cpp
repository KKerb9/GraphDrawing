#include "KleinProjection.h"

#include <algorithm>
#include <cmath>
#include <limits>
#include <utility>

#include "../metrics/Metrics.h"
#include "../spaces/HyperbolicSpace.h"
#include "../spaces/KleinSpace.h"

namespace gd {

namespace {

constexpr ld EPS_KLEIN = 1e-12L;

ld dot(const Pt& a, const Pt& b) {
	ld res = 0.0L;
	for (size_t i = 0; i < a.size(); i++) res += a[i] * b[i];
	return res;
}

ld norm(const Pt& p) {
	return std::sqrtl(dot(p, p));
}

ld metricValue(const Metrics& metrics, int32_t index) {
	if (index == 0) return metrics.minVertexDist;
	if (index == 1) return metrics.minEdgeVertexDist;
	if (index == 2) return metrics.minAngle;
	return metrics.edgeCrossings;
}

ld metricWeight(int32_t index) {
	return index == 3 ? -1.0L : 1.0L;
}

} // namespace

KleinProjection::KleinProjection(KleinProjectionMode mode, uint32_t seed, int32_t candidates)
	: _mode(mode), _seed(seed), _candidates(candidates),
	_name(mode == KleinProjectionMode::Orthogonal ? "kleinOrthogonal" : "kleinBestView") {}

std::string KleinProjection::name() const {
	return _name;
}

ProjectionResult KleinProjection::project(
		const Embedding& emb,
		const Space& space,
		const std::vector<int32_t>& figSize,
		int32_t finalDim) const {
	if (space.name() != "hyperbolic") {
		throw ProjectionError("KleinProjection::project: only Hyperbolic space is supported");
	}
	if (emb.dimension() < 2 || finalDim != 2) {
		throw ProjectionError("KleinProjection::project: only Hn to 2D projection is supported");
	}
	if (figSize.size() != 2 || figSize[0] <= 0 || figSize[1] <= 0) {
		throw ProjectionError("KleinProjection::project: figSize must contain two positive sides");
	}
	if (_candidates <= 0) {
		throw ProjectionError("KleinProjection::project: cameraCandidates must be positive");
	}

	const ld radius = static_cast<ld>(std::min(figSize[0], figSize[1])) / 2.0L;
	std::vector<Pt> kleinCoords = toKleinCoords(emb, space);
	std::vector<Pt> bestCamera = orthogonalCamera(emb.dimension());
	if (_mode == KleinProjectionMode::BestView && _candidates > 1 && !kleinCoords.empty()) {
		std::vector<std::vector<Pt>> cameras;
		cameras.reserve(_candidates);
		cameras.push_back(bestCamera);

		std::mt19937_64 rng(_seed);
		for (int32_t i = 1; i < _candidates; i++) {
			cameras.push_back(randomCamera(emb.dimension(), rng));
		}

		std::vector<std::vector<ld>> values(4, std::vector<ld>(_candidates));
		KleinSpace drawingSpace(2, radius);
		for (int32_t i = 0; i < _candidates; i++) {
			Embedding current(emb.getGraph(), projectWithCamera(kleinCoords, cameras[i], radius));
			Metrics metrics = computeMetrics(current, drawingSpace);
			for (int32_t metric = 0; metric < 4; metric++) {
				values[metric][i] = metricValue(metrics, metric);
			}
		}

		std::vector<ld> scores(_candidates, 0.0L);
		for (int32_t metric = 0; metric < 4; metric++) {
			ld minValue = *std::min_element(values[metric].begin(), values[metric].end());
			ld maxValue = *std::max_element(values[metric].begin(), values[metric].end());
			if (minValue == maxValue) {
				for (ld& score : scores) score += std::fabsl(metricWeight(metric));
				continue;
			}
			for (int32_t i = 0; i < _candidates; i++) {
				ld normalized = (values[metric][i] - minValue) / (maxValue - minValue);
				scores[i] += metricWeight(metric) > 0.0L
					? metricWeight(metric) * normalized
					: std::fabsl(metricWeight(metric)) * (1.0L - normalized);
			}
		}
		int32_t bestIndex = 0;
		for (int32_t i = 1; i < _candidates; i++) {
			if (scores[i] > scores[bestIndex]) bestIndex = i;
		}
		bestCamera = cameras[bestIndex];
	}

	std::vector<Pt> coords = projectWithCamera(kleinCoords, bestCamera, radius);
	Embedding result = coords.empty()
		? Embedding(emb.getGraph(), finalDim)
		: Embedding(emb.getGraph(), coords);
	return ProjectionResult{std::move(result), std::make_unique<KleinSpace>(finalDim, radius)};
}

std::vector<Pt> KleinProjection::toKleinCoords(const Embedding& emb, const Space& space) const {
	const auto* hyperbolicSpace = dynamic_cast<const HyperbolicSpace*>(&space);
	if (hyperbolicSpace == nullptr) {
		throw ProjectionError("KleinProjection::toKleinCoords: invalid Hyperbolic space");
	}
	std::vector<Pt> res(emb.size(), Pt(emb.dimension(), 0.0L));
	for (int32_t i = 0; i < emb.size(); i++) {
		const Pt& x = emb.getCoord(i);
		if (!hyperbolicSpace->isValid(x)) {
			throw ProjectionError("KleinProjection::toKleinCoords: invalid hyperbolic point");
		}
		res[i] = hyperbolicSpace->toKlein(x);
		if (norm(res[i]) > 1.0L - EPS_KLEIN) {
			throw ProjectionError("KleinProjection::toKleinCoords: point is too close to Klein boundary");
		}
	}
	return res;
}

std::vector<Pt> KleinProjection::projectWithCamera(
		const std::vector<Pt>& kleinCoords,
		const std::vector<Pt>& camera,
		ld radius) const {
	std::vector<Pt> res(kleinCoords.size(), Pt(2, 0.0L));
	for (size_t i = 0; i < kleinCoords.size(); i++) {
		Pt point = {dot(camera[0], kleinCoords[i]), dot(camera[1], kleinCoords[i])};
		ld pointNorm = norm(point);
		if (pointNorm > 1.0L - EPS_KLEIN) {
			if (pointNorm > 1.0L + EPS_KLEIN) {
				throw ProjectionError("KleinProjection::projectWithCamera: point is outside Klein disk");
			}
			point = point / pointNorm * (1.0L - EPS_KLEIN);
		}
		res[i] = point * radius;
	}
	return res;
}

std::vector<Pt> KleinProjection::orthogonalCamera(int32_t dim) const {
	std::vector<Pt> camera(2, Pt(dim, 0.0L));
	camera[0][0] = 1.0L;
	camera[1][1] = 1.0L;
	return camera;
}

std::vector<Pt> KleinProjection::randomCamera(int32_t dim, std::mt19937_64& rng) const {
	std::normal_distribution<ld> distribution(0.0L, 1.0L);
	while (true) {
		Pt first(dim, 0.0L), second(dim, 0.0L);
		for (int32_t i = 0; i < dim; i++) {
			first[i] = distribution(rng);
			second[i] = distribution(rng);
		}
		ld firstNorm = norm(first);
		if (firstNorm <= std::numeric_limits<ld>::epsilon()) continue;
		first = first / firstNorm;
		second = second - first * dot(first, second);
		ld secondNorm = norm(second);
		if (secondNorm <= std::numeric_limits<ld>::epsilon()) continue;
		second = second / secondNorm;
		return {first, second};
	}
}

} // namespace gd
