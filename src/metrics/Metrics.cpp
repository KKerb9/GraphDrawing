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

	res.volume = 0;

	if (n > 0) {
		Pt mns = pts[0], mxs = pts[0];
		for (const Pt &p : pts) {
			for (int j = 0; j < dim; j++) {
				mns[j] = std::min(mns[j], p[j]);
				mxs[j] = std::max(mxs[j], p[j]);
			}
		}
                std::vector<int32_t> figSize(dim);
		for (int j = 0; j < dim; j++) {
			figSize[j] = mxs[j] - mns[j];
		}
                res.volume = space.volume(figSize);
	}

	res.edgeCrossings = 0;
	res.minAngle = 0;
	res.maxAngle = 0;
        res.density = (cmp.sgn(res.volume) > 0) ? n / res.volume : 0;
        res.minEdgeVertexDist = 0;  // TODO: пока что в пуанкаре считается по обычным прямым ребрам, без учета дуг


	auto dist = [ & ](const Pt &a, const Pt &b) -> ld {
		ld res = 0;
		for (int i = 0; i < dim; i++) {
			res += (a[i] - b[i]) * (a[i] - b[i]);
		}
		return std::sqrtl(res);
	};

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

        auto vec = [ & ](const Pt &a, const Pt &b) -> ld {
                // NOTE: только в 2мерном
                return a[0] * b[1] - a[1] * b[0];
        };

        auto scal = [ & ](const Pt &a, const Pt &b) -> ld {
                ld res = 0;
                for (int i = 0; i < dim; i++) res += a[i] * b[i];
                return res;
        };
        
        auto is_pt_on_seg = [ & ](const Pt &a, const Pt &b, const Pt &p) -> bool {
                if (a == b) {
                        return a == p;
                }
                Pt ab = b - a, ap = p - a, ba = a - b, bp = p - b;
                if (cmp.sgn(vec(ab, ap)) != 0) {
                        return false;
                }
                if (cmp.sgn(scal(ap, ab)) >= 0 && cmp.sgn(scal(ba, bp)) >= 0) {
                        return true;
                }
                return false;
        };
        
        auto pt_to_seg = [ & ](const Pt &a, const Pt &b, const Pt &p) -> ld {
                if (a == b) return dist(a, p);
                if (is_pt_on_seg(a, b, p)) {
                        return 0;
                }
                Pt ab = b - a, ap = p - a, ba = a - b, bp = p - b;
                if (cmp.sgn(scal(ab, ap)) >= 0 && cmp.sgn(scal(ba, bp)) >= 0) {
                        ld fa = a[1] - b[1], fb = b[0] - a[0], fc = -1 * a[1] * b[0] + b[1] * a[0];
                        return abs((fa * p[0] + fb * p[1] + fc) / std::sqrtl(fa * fa + fb * fb));
                } else {
                        return std::min(dist(a, p), dist(b, p));
                }
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
                res.minEdgeVertexDist = 1e18;
                for (int i = 0; i < m; i++) {
                        for (int v = 0; v < n; v++) {
                                if (v == edges[i].first || v == edges[i].second) continue;
                                ld d = pt_to_seg(pts[edges[i].first], pts[edges[i].second], pts[v]);
                                res.minEdgeVertexDist = std::min(res.minEdgeVertexDist, d);
                        }
                }
                if (res.minEdgeVertexDist == 1e18) {
                        res.minEdgeVertexDist = 0;
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
