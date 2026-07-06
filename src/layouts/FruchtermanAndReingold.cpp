#include "FruchtermanAndReingold.h"

#include <algorithm>
#include <cmath>
#include <random>

namespace gd {

FruchtermanAndReingoldLayout::FruchtermanAndReingoldLayout() : LayoutAlgorithm("far") {
        ITERS = 100;
        C = 1.0;
}

FruchtermanAndReingoldLayout::FruchtermanAndReingoldLayout(int32_t ITERS, ld C) : LayoutAlgorithm("far") {
        this->ITERS = ITERS;
        this->C = C;
}

void FruchtermanAndReingoldLayout::computeLayout(
        Embedding& emb,
        const Space& space,
        const BorderPolicy& borderPolicy) const {
        computeLayoutTest(emb, space, borderPolicy);
}

void FruchtermanAndReingoldLayout::computeLayoutTest(
        Embedding& emb,
        const Space& space,
        const BorderPolicy& borderPolicy) const {

        const ld EPS = 1e-12;
        Comparator cmp(EPS);

        int n = emb.size();
        int dim = space.dimension();
        assert(dim == emb.dimension());
        ld vol = borderPolicy.domainVolume(space);
        ld k = cmp.sgn(vol) != -1 ? C * std::powl(vol / n, 1.0L / dim) : 1;  // так в статье предлагали

        auto f_a = [ & ](ld x) -> ld {
                return x * x / k;
        };

        auto f_r = [ & ](ld x) -> ld {
                return k * k / x;
        };

        ld T = k;  // TODO: поменять на нормальную температуру

        auto cool = [ & ](ld T, int it) -> ld {
                return T * ((it * 4 <= ITERS) ? 0.85 : 0.95);
        };

        /* если слишком кучно, то немного расшатать точки, что бы не застревать
        ld jitterRadius = std::max(1e-6L, radius / 50.0L);
        std::mt19937 rng(1543);
        std::uniform_real_distribution<ld> unitDist(-1.0L, 1.0L);
        std::uniform_real_distribution<ld> lenDist(0.0L, 1.0L);

        for (int i = 0; i < n; i++) {
                Pt jitter(dim, 0.0L);
                for (int c = 0; c < dim; c++) {
                        jitter[c] = unitDist(rng);
                }
                ld jitterN = space.tangentNorm(emb.getCoord(i), jitter);
                if (cmp.sgn(jitterN) == 0) continue;
                Pt step = jitter / jitterN * (lenDist(rng) * jitterRadius);
                Pt p = space.expMap(emb.getCoord(i), step);
                emb.setPos(i, space.normalizePoint(p, figSize));
        }
        */

        for (int it = 0; it < ITERS; it++) {
                std::vector<Pt> disp(n, std::vector<ld>(dim));
                for (int i = 0; i < n; i++) {
                        for (int j = i + 1; j < n; j++) {
                                Pt v1 = space.logMap(emb.getCoord(i), emb.getCoord(j));
                                Pt v2 = space.logMap(emb.getCoord(j), emb.getCoord(i));
                                ld v1N = space.tangentNorm(emb.getCoord(i), v1);
                                ld v2N = space.tangentNorm(emb.getCoord(j), v2);
                                if (cmp.sgn(v1N)) disp[i] -= v1 / v1N * f_r(v1N);
                                if (cmp.sgn(v2N)) disp[j] -= v2 / v2N * f_r(v2N);
                        }
                }
                for (const auto &[i, j] : emb.getEdges()) {
                        Pt v1 = space.logMap(emb.getCoord(i), emb.getCoord(j));
                        Pt v2 = space.logMap(emb.getCoord(j), emb.getCoord(i));
                        ld v1N = space.tangentNorm(emb.getCoord(i), v1);
                        ld v2N = space.tangentNorm(emb.getCoord(j), v2);
                        if (cmp.sgn(v1N)) disp[i] += v1 / v1N * f_a(v1N);
                        if (cmp.sgn(v2N)) disp[j] += v2 / v2N * f_a(v2N);
                }
                for (int i = 0; i < n; i++) {
                        ld dispN = space.tangentNorm(emb.getCoord(i), disp[i]);
                        if (cmp.sgn(dispN) == 0) continue;

                        Pt step = (disp[i] / dispN) * std::min(dispN, T);
                        Pt newPos = space.expMap(emb.getCoord(i), step);
                        newPos = borderPolicy.normalizePoint(space, newPos);
                        emb.setPos(i, newPos);
                }
                T = cool(T, it);
        }
}

void FruchtermanAndReingoldLayout::computeLayoutEuclideanOld(
        Embedding& emb,
        const Space& space,
        const std::vector<int32_t>& figSize) const {

        const ld EPS = 1e-12;
        Comparator cmp(EPS);

        // ll area = space.area(figSize);
        ll area = 0;
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