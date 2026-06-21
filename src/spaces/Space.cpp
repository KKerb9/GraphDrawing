#include "Space.h"

#include <algorithm>
#include <cmath>

#include "EuclideanSpace.h"
#include "HyperbolicSpace.h"

namespace gd {

ld Space::determinantBareiss(std::vector<std::vector<ld>> a) {
	const ld EPS = 1e-18L;
	int32_t n = static_cast<int32_t>(a.size());
	if (n == 0) return 1.0L;
	for (int32_t i = 0; i < n; i++) {
		if (static_cast<int32_t>(a[i].size()) != n) {
			throw SpaceError("determinant: matrix must be square");
		}
	}

	int32_t sign = 1;
	ld prev = 1.0L;

	for (int32_t k = 0; k + 1 < n; k++) {
		int32_t pivot = -1;
		ld pivotAbs = 0.0L;
		for (int32_t i = k; i < n; i++) {
			ld curAbs = std::fabsl(a[i][k]);
			if (curAbs > pivotAbs) {
				pivotAbs = curAbs;
				pivot = i;
			}
		}
		if (pivot == -1 || pivotAbs < EPS) return 0.0L;
		if (pivot != k) {
			std::swap(a[pivot], a[k]);
			sign = -sign;
		}

		ld curPivot = a[k][k];
		if (std::fabsl(curPivot) < EPS) return 0.0L;
		if (std::fabsl(prev) < EPS) return 0.0L;

		for (int32_t i = k + 1; i < n; i++) {
			for (int32_t j = k + 1; j < n; j++) {
				ld num = a[i][j] * curPivot - a[i][k] * a[k][j];
				a[i][j] = num / prev;
			}
			a[i][k] = 0.0L;
		}
		prev = curPivot;
	}

	ld det = sign * a[n - 1][n - 1];
	if (std::fabsl(det) < EPS) return 0.0L;
	return det;
}

bool Space::areEuclideanSegmentsCrossing2D(const Pt& a, const Pt& b, const Pt& c, const Pt& d) {
        Comparator cmp;

        auto scal = [ & ](const Pt &a, const Pt &b) -> ld {
		return a[0] * b[0] + a[1] * b[1];
	};

	auto vec = [ & ](const Pt &a, const Pt &b) -> ld {
		return a[0] * b[1] - a[1] * b[0];
	};

        auto isPtOnSeg = [ & ](const Pt &a, const Pt &b, const Pt &p) -> bool {
		if (a == b) return true;
		Pt ab = b - a, ap = p - a, ba = a - b, bp = p - b;
		if (cmp.sgn(vec(ab, ap)) != 0) return false;
		return (cmp.sgn(scal(ap, ab)) >= 0 && cmp.sgn(scal(ba, bp)) >= 0);
	};

        if (a == c || a == d || b == c || b == d) return false;
        Pt ab = b - a, ac = c - a, ad = d - a;
        Pt db = b - d, da = a - d, dc = c - d;
        if (isPtOnSeg(a, b, c) || isPtOnSeg(a, b, d) || isPtOnSeg(c, d, a) || isPtOnSeg(c, d, b)) return true;
        return !(cmp.sgn(vec(ab, ac)) == cmp.sgn(vec(ab, ad)) || cmp.sgn(vec(dc, da)) == cmp.sgn(vec(dc, db)));
}

SpacePtr createSpace(const std::string& spaceName, int32_t dim) {
	if (spaceName == "euclidean") {
		return std::make_unique<EuclideanSpace>(dim);
	}
	if (spaceName == "hyperbolic") {
		return std::make_unique<HyperbolicSpace>(dim);
	}
	throw SpaceError("createSpace: unknown space name: " + spaceName);
}

} // namespace gd
