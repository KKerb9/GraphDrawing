#pragma once

#include "../core/Embedding.h"
#include "../core/Errors.h"
#include "../spaces/Space.h"
#include "../core/template.h"

namespace gd {

class MetricsError : public GraphDrawingError {
public:
	using GraphDrawingError::GraphDrawingError;
};

struct Metrics {
	ld volume;
	ld minVertexDist;  // минимальное расстояние между вершинами
	ld maxVertexDist;  // максимальное расстояние между вершинами
	ld avgVertexDist;  // среднее расстояние между вершинами
	int32_t edgeCrossings;  // количество пересечений ребер
	ld minAngle;  // минимальный угол между ребрами
	ld maxAngle;  // максимальный угол между ребрами
	ld density; // плотность графа
};

Metrics computeMetrics(const Embedding& emb, const Space& space, const std::vector<int32_t>& figSize) {
	Metrics res;
	Comparator cmp;
	int32_t n = emb.size();
	auto graph = emb.getGraph();
	auto edges = graph.getEdges();
	int32_t m = edges.size();
	int32_t dim = emb.dimension();
	std::vector<ld> mns = figSize / 2;
	std::vector<ld> mxs = figSize / 2;
	for (auto el : mxs) el = -el;
	auto pts = emb.getCoords();
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
	res.minVertexDist = 1e18;
	res.maxVertexDist = 0;
	res.avgVertexDist = 0;
	res.edgeCrossings = 0;
	res.minAngle = 1e18;
	res.maxAngle = 0;
	res.density = 0;

	auto scal = [ & ](const pt &a, const pt &b) -> ld {
		ld res = 0;
		for (int i = 0; i < dim; i++) {
			res += a[i] * b[i];
		}
		return res;
	};

	auto vec = [ & ](const Pt &a, const Pt &b) -> ld {
		if (dim == 2) {
			return a[0] * b[1] - a[1] * b[0]};
		}
		if (dim == 3) {
			return Pt{
				a[1] * b[2] - a[2] * b[1],
				a[2] * b[0] - a[0] * b[2],
				a[0] * b[1] - a[1] * b[0]
			};
		}
	};

	auto isPtOnSeg = [ & ](const pt &a, const pt &b, const pt &p) -> bool {
		if (a == b) return true;
		pt ab = b - a, ap = p - a, ba = a - b, bp = p - b;
		if (cmp.sgn(vec(ab, ap)) != 0) return false;
		return (cmp.sgn(scal(ap, ab)) >= 0 && cmp.sgn(scal(ba, bp)) >= 0);
	};

	auto areSegsCrossing = [ & ](const pt &a, const pt &b, const pt &c, const pt &d) -> bool {
		pt ab = b - a, ac = c - a, ad = d - a;
		pt db = b - d, da = a - d, dc = c - d;
		if (isPtOnSeg(a, b, c) || isPtOnSeg(a, b, d) || isPtOnSeg(c, d, a) || isPtOnSeg(c, d, b)) return true;
		return !(cmp.sgn(vec(ab, ac)) == cmp.sgn(vec(ab, ad)) || cmp.sgn(vec(dc, da)) == cmp.sgn(vec(dc, db)));
	};

	auto norm = [ & ](const pt &a) -> ld {
		return sqrtl(scal(a, a));
	};

	auto angle = [ & ](const pt &a, const pt &b) -> ld {
		return acosl(scal(a, b) / (norm(a) * norm(b)));
	};

	for (int i = 0; i < m; i++) {
		for (int j = i + 1; j < m; j++) {
			if (areSegsCrossing(pts[edges[i].first], pts[edges[i].second], pts[edges[j].first], pts[edges[j].second])) {
				res.edgeCrossings++;
			}
			if (edges[i].first == edges[j].first) {

			} else if (edges[i].first == edges[j].second) {
			
			} else if (edges[i].second == edges[j].first) {

			} else if (edges[i].second == edges[j].second) {

			}
		}
	}
	for (int i = 0; i < n; i++) {
		
	}

} // namespace gd
