#include "Metrics.h"

#include <cmath>

namespace gd {

Metrics computeMetrics(const Embedding& emb, const Space& space, const std::vector<int32_t>& figSize) {
	Metrics res;
	Comparator cmp;
	int32_t n = emb.size();
	auto graph = emb.getGraph();
	auto edges = graph.edges();
	int32_t m = edges.size();
	int32_t dim = emb.dimension();
	auto pts = emb.getCoords();

        std::vector<ld> mns(dim);
        std::vector<ld> mxs(dim);
        for (int j = 0; j < dim; j++) {
                mns[j] = (ld)figSize[j] / 2;
                mxs[j] = -(ld)figSize[j] / 2;
        }
        for (const auto &p : pts) {
                for (int j = 0; j < dim; j++) {
                        mns[j] = std::min(mns[j], p[j]);
                        mxs[j] = std::max(mxs[j], p[j]);
                }
        }
        res.volume = 1.0;
        for (int j = 0; j < dim; j++) {
                res.volume *= (mxs[j] - mns[j]);
        }

	res.edgeCrossings = 0;
	res.minAngle = 0;
	res.maxAngle = 0;
        res.density = 0;

	if (n < 2) {
		res.minVertexDist = 0;
		res.maxVertexDist = 0;
		res.avgVertexDist = 0;
		return res;
	}

	res.minVertexDist = 1e18;
	res.maxVertexDist = 0;
	res.avgVertexDist = 0;
	res.minAngle = 1e18;

	auto scal = [ & ](const Pt &a, const Pt &b) -> ld {
		ld s = 0;
		for (int i = 0; i < dim; i++) {
			s += a[i] * b[i];
		}
		return s;
	};

	auto vec = [ & ](const Pt &a, const Pt &b) -> ld {
		if (dim == 2) {
			return a[0] * b[1] - a[1] * b[0];
		} else {
			throw MetricsError("vecProd: 3D space");
		}
		// if (dim == 3) {
		// 	return Pt{
		// 		a[1] * b[2] - a[2] * b[1],
		// 		a[2] * b[0] - a[0] * b[2],
		// 		a[0] * b[1] - a[1] * b[0]
		// 	};
		// }
	};

	auto isPtOnSeg = [ & ](const Pt &a, const Pt &b, const Pt &p) -> bool {
		if (a == b) return true;
		Pt ab = b - a, ap = p - a, ba = a - b, bp = p - b;
		if (cmp.sgn(vec(ab, ap)) != 0) return false;
		return (cmp.sgn(scal(ap, ab)) >= 0 && cmp.sgn(scal(ba, bp)) >= 0);
	};

	auto areSegsCrossing = [ & ](const Pt &a, const Pt &b, const Pt &c, const Pt &d) -> bool {
                if (a == c || a == d || b == c || b == d) return false;
		Pt ab = b - a, ac = c - a, ad = d - a;
		Pt db = b - d, da = a - d, dc = c - d;
		if (isPtOnSeg(a, b, c) || isPtOnSeg(a, b, d) || isPtOnSeg(c, d, a) || isPtOnSeg(c, d, b)) return true;
		return !(cmp.sgn(vec(ab, ac)) == cmp.sgn(vec(ab, ad)) || cmp.sgn(vec(dc, da)) == cmp.sgn(vec(dc, db)));
	};

	auto norm = [ & ](const Pt &a) -> ld {
		return sqrtl(scal(a, a));
	};

	auto angle = [ & ](const Pt &a, const Pt &b) -> ld {
		return std::fabs(atan2(vec(a, b), scal(a, b)));
	};

	for (int i = 0; i < m; i++) {
		for (int j = i + 1; j < m; j++) {
			if (areSegsCrossing(pts[edges[i].first], pts[edges[i].second], pts[edges[j].first], pts[edges[j].second])) {
				res.edgeCrossings++;
			}
			if (edges[i].first == edges[j].first) {
				res.minAngle = std::min(res.minAngle, angle(pts[edges[i].second] - pts[edges[i].first], pts[edges[j].second] - pts[edges[j].first]));
				res.maxAngle = std::max(res.maxAngle, angle(pts[edges[i].second] - pts[edges[i].first], pts[edges[j].second] - pts[edges[j].first]));
				// res.avgAngle += angle(pts[edges[i].second] - pts[edges[i].first], pts[edges[j].second] - pts[edges[j].first]);
			} else if (edges[i].first == edges[j].second) {
				res.minAngle = std::min(res.minAngle, angle(pts[edges[i].second] - pts[edges[i].first], pts[edges[j].first] - pts[edges[j].second]));
				res.maxAngle = std::max(res.maxAngle, angle(pts[edges[i].second] - pts[edges[i].first], pts[edges[j].first] - pts[edges[j].second]));
				// res.avgAngle += angle(pts[edges[i].second] - pts[edges[i].first], pts[edges[j].first] - pts[edges[j].second]);
			} else if (edges[i].second == edges[j].first) {
				res.minAngle = std::min(res.minAngle, angle(pts[edges[i].first] - pts[edges[i].second], pts[edges[j].second] - pts[edges[j].first]));
				res.maxAngle = std::max(res.maxAngle, angle(pts[edges[i].first] - pts[edges[i].second], pts[edges[j].second] - pts[edges[j].first]));
				// res.avgAngle += angle(pts[edges[i].first] - pts[edges[i].second], pts[edges[j].second] - pts[edges[j].first]);
			} else if (edges[i].second == edges[j].second) {
				res.minAngle = std::min(res.minAngle, angle(pts[edges[i].first] - pts[edges[i].second], pts[edges[j].first] - pts[edges[j].second]));
				res.maxAngle = std::max(res.maxAngle, angle(pts[edges[i].first] - pts[edges[i].second], pts[edges[j].first] - pts[edges[j].second]));
				// res.avgAngle += angle(pts[edges[i].first] - pts[edges[i].second], pts[edges[j].first] - pts[edges[j].second]);
			}
		}
	}
	for (int i = 0; i < n; i++) {
		for (int j = i + 1; j < n; j++) {
			res.minVertexDist = std::min(res.minVertexDist, space.dist(pts[i], pts[j]));
			res.maxVertexDist = std::max(res.maxVertexDist, space.dist(pts[i], pts[j]));
			res.avgVertexDist += space.dist(pts[i], pts[j]);
		}
	}
	res.avgVertexDist /= n * (n - 1) / 2;
	return res;
}

} // namespace gd
