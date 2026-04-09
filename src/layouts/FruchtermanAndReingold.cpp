#include "FruchtermanAndReingold.h"

#include <algorithm>
#include <cassert>

namespace gd {

FruchtermanAndReingoldLayout::FruchtermanAndReingoldLayout(int32_t ITERS, ld C)
	: LayoutAlgorithm("far"),
	  ITERS(ITERS),
	  C(C) {}

void FruchtermanAndReingoldLayout::computeLayout(
		Embedding& emb,
		const Space& space,
		const std::vector<int32_t>& figSize) const {

        const ld EPS = 1e-12;
        Comparator cmp(EPS);
        
        ll area = space.area(figSize);
        int n = emb.size();
        int dim = space.dimension();
        assert(dim == emb.dimension());
        ld k = C * std::sqrt((ld)area / n);

        auto f_a = [ & ](ld x) -> ld {
                return x * x / k;
        };

        auto f_r = [ & ](ld x) -> ld {
                return k * k / x;
        };

        ld T = (ld)*std::min_element(figSize.begin(), figSize.end()) / 5;

        auto cool = [ & ](ld T, int it) -> ld {
                return T * ((it * 4 <= ITERS) ? 0.85 : 0.95);
        };

        for (int it = 0; it < ITERS; it++) {
                std::vector<Pt> disp(n, std::vector<ld>(dim));  // displacement lol
                for (int i = 0; i < n; i++) {
                        for (int j = i + 1; j < n; j++) {
                                Pt v1 = emb.getCoord(i) - emb.getCoord(j);
                                Pt v2 = emb.getCoord(j) - emb.getCoord(i);
                                ld v1N = space.norm(v1);
                                ld v2N = space.norm(v2);
                                if (cmp.sgn(v1N)) disp[i] += v1 / v1N * f_r(v1N);
                                if (cmp.sgn(v2N)) disp[j] += v2 / v2N * f_r(v2N);
                        }
                }
                for (const auto &[i, j] : emb.getEdges()) {
                        Pt v = emb.getCoord(i) - emb.getCoord(j);
                        ld vN = space.norm(v);
                        if (cmp.sgn(vN) == 0) continue;

                        disp[i] -= v / vN * f_a(vN);
                        disp[j] += v / vN * f_a(vN);
                }
                for (int i = 0; i < n; i++) {
                        ld dispN = space.norm(disp[i]);
                        if (cmp.sgn(dispN) == 0) continue;

                        Pt newPos = emb.getCoord(i) + (disp[i] / dispN) * std::min(dispN, T);
                        for (int c = 0; c < dim; c++) {
                                newPos[c] = std::min((ld)figSize[c] / 2, std::max(-(ld)figSize[c] / 2, newPos[c]));
                        }
                        emb.setPos(i, newPos);
                }
                T = cool(T, it);
        }
}

}