#include "Metrics.h"

#include <cmath>

namespace gd {

Metrics computeMetrics(const Embedding& emb, const Space& space) {
	Metrics res;
	Comparator cmp;
	int32_t n = emb.size();
	auto graph = emb.getGraph();
	auto edges = graph.edges();
	int32_t m = edges.size();
	int32_t dim = emb.dimension();
	auto pts = emb.getCoords();

	res.volume = 0;  // NOTE: пока что закостылено на простую евклидову площадь
	if (n > 0) {
		Pt mns = pts[0], mxs = pts[0];
		for (const Pt &p : pts) {
			for (int j = 0; j < dim; j++) {
				mns[j] = std::min(mns[j], p[j]);
				mxs[j] = std::max(mxs[j], p[j]);
			}
		}
		res.volume = 1;
		for (int j = 0; j < dim; j++) {
			res.volume *= mxs[j] - mns[j];
		}
	}

	res.edgeCrossings = 0;
	res.minAngle = 0;
	res.maxAngle = 0;
        res.density = (cmp.sgn(res.volume) > 0) ? n / res.volume : 0;

	if (n < 2) {
		res.minVertexDist = 0;
		res.maxVertexDist = 0;
		res.avgVertexDist = 0;
		return res;
	}

	res.minVertexDist = 1e18;
	res.maxVertexDist = 0;  // NOTE: так же пока что дисты закостылены на евклида
	res.avgVertexDist = 0;
	if (dim == 2) {
		res.minAngle = 1e18;
	}

	auto angle = [ & ](const Pt &center, const Pt &a, const Pt &b) -> ld {
		Pt u = space.logMap(center, a);
		Pt v = space.logMap(center, b);
		ld uNorm = space.tangentNorm(center, u);
		ld vNorm = space.tangentNorm(center, v);
		if (cmp.sgn(uNorm) == 0 || cmp.sgn(vNorm) == 0) return 0;
		ld sumNorm = space.tangentNorm(center, u + v);
		ld scal = (sumNorm * sumNorm - uNorm * uNorm - vNorm * vNorm) / 2;
		ld cos = std::max(-1.0L, std::min(1.0L, scal / uNorm / vNorm));
		return std::acosl(cos);
	};

	auto dist = [ & ](const Pt &a, const Pt &b) -> ld {
		ld res = 0;
		for (int i = 0; i < dim; i++) {
			res += (a[i] - b[i]) * (a[i] - b[i]);
		}
		return std::sqrtl(res);
	};

	if (dim == 2) {
		for (int i = 0; i < m; i++) {
			for (int j = i + 1; j < m; j++) {
				if (space.areGeodesicSegmentsCrossing(pts[edges[i].first], pts[edges[i].second], pts[edges[j].first], pts[edges[j].second])) {
					res.edgeCrossings++;
				}
				if (edges[i].first == edges[j].first) {
					res.minAngle = std::min(res.minAngle, angle(pts[edges[i].first], pts[edges[i].second], pts[edges[j].second]));
					res.maxAngle = std::max(res.maxAngle, angle(pts[edges[i].first], pts[edges[i].second], pts[edges[j].second]));
					// res.avgAngle += angle(pts[edges[i].second] - pts[edges[i].first], pts[edges[j].second] - pts[edges[j].first]);
				} else if (edges[i].first == edges[j].second) {
					res.minAngle = std::min(res.minAngle, angle(pts[edges[i].first], pts[edges[i].second], pts[edges[j].first]));
					res.maxAngle = std::max(res.maxAngle, angle(pts[edges[i].first], pts[edges[i].second], pts[edges[j].first]));
					// res.avgAngle += angle(pts[edges[i].second] - pts[edges[i].first], pts[edges[j].first] - pts[edges[j].second]);
				} else if (edges[i].second == edges[j].first) {
					res.minAngle = std::min(res.minAngle, angle(pts[edges[i].second], pts[edges[i].first], pts[edges[j].second]));
					res.maxAngle = std::max(res.maxAngle, angle(pts[edges[i].second], pts[edges[i].first], pts[edges[j].second]));
					// res.avgAngle += angle(pts[edges[i].first] - pts[edges[i].second], pts[edges[j].second] - pts[edges[j].first]);
				} else if (edges[i].second == edges[j].second) {
					res.minAngle = std::min(res.minAngle, angle(pts[edges[i].second], pts[edges[i].first], pts[edges[j].first]));
					res.maxAngle = std::max(res.maxAngle, angle(pts[edges[i].second], pts[edges[i].first], pts[edges[j].first]));
					// res.avgAngle += angle(pts[edges[i].first] - pts[edges[i].second], pts[edges[j].first] - pts[edges[j].second]);
				}
			}
		}
	}
	for (int i = 0; i < n; i++) {
		for (int j = i + 1; j < n; j++) {
			ld d = dist(pts[i], pts[j]);
			res.minVertexDist = std::min(res.minVertexDist, d);
			res.maxVertexDist = std::max(res.maxVertexDist, d);
			res.avgVertexDist += d;
		}
	}
	res.avgVertexDist /= n * (n - 1) / 2;
	return res;
}

} // namespace gd
