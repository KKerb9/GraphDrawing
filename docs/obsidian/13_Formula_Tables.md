---
title: Formula Tables
tags: [graphdrawing, formulas, reference]
source: ../../PROJECT_REPORT.md
---

# Проверочные таблицы формул

Навигация: [[00_Index|Индекс]] | назад [[12_Limitations_Roadmap|Ограничения и развитие]]

Эта заметка - компактный справочник. Подробные выводы находятся в заметках:

- [[05_EuclideanSpace|EuclideanSpace]]
- [[06_HyperbolicSpace|HyperbolicSpace]]
- [[07_PoincareSpace|PoincareSpace]]
- [[08_Layout_Algorithms|Алгоритмы раскладки]]
- [[09_Projections|Проекции]]
- [[10_Metrics|Метрики]]

## EuclideanSpace

| Метод | Формула/поведение |
|---|---|
| `isValid(x)` | `size(x) == d`. |
| `dist(a,b)` | `sqrt(sum_i (a_i-b_i)^2)`. |
| `norm(v)` | `sqrt(sum_i v_i^2)`. |
| `logMap(from,to)` | `to - from`. |
| `expMap(from,v)` | `from + v`. |
| `tangentNorm(at,v)` | `norm(v)`. |
| `normalizePoint(p,FS)` | Clamp по каждой оси в `[-FS_i/2, FS_i/2]`. |
| `volume(FS)` | `sqrt(det(diag(FS_i^2)))`. |
| `areGeodesicSegmentsCrossing` | Только 2D, прямые отрезки. |

## HyperbolicSpace

| Метод | Формула/поведение |
|---|---|
| `isValid(x)` | `size(x)==d` и все координаты finite. |
| `lift(x)` | `(sqrt(1+norm(x)^2), x)`. |
| `lorentzProd(A,B)` | `-A_0B_0 + sum_i A_iB_i`. |
| `dist(a,b)` | `arcosh(max(-<lift(a),lift(b)>_L,1))`. |
| `norm(v)` | Евклидова координатная норма. |
| `tangentToDefault(x,v)` | `((x·v)/sqrt(1+norm(x)^2), v)`. |
| `defaultToTangent(W)` | Отбрасывает `W_0`. |
| `logMap(x,y)` | `d(Y-alpha X)/sqrt(<Y-alpha X,Y-alpha X>_L)`, затем spatial part. |
| `expMap(x,v)` | `cosh(vNorm)X + sinh(vNorm)/vNorm V`, затем spatial part. |
| `tangentNorm(x,v)` | `sqrt(max(<V,V>_L,0))`. |
| `normalizePoint(p,FS)` | Ограничение геодезическим шаром радиуса `min(FS)/2`. |
| `volume(FS)` | Объем касательного параллелепипеда в origin через Gram determinant. |
| `areGeodesicSegmentsCrossing` | Только 2D, переход в Klein `p/sqrt(1+norm(p)^2)`. |

## PoincareSpace

| Метод | Формула/поведение |
|---|---|
| `isValid(p)` | `size(p)==2`, finite, `norm(p) < R`. |
| `toSmallDisk(p)` | `p/R`. |
| `toDefDisk(x)` | `R*x`. |
| `mobiusAdd(a,b)` | `((1+2<a,b>+norm(b)^2)a + (1-norm(a)^2)b)/(1+2<a,b>+norm(a)^2 norm(b)^2)`. |
| `dist(A,B)` | `2 atanh(min(norm((-A/R) ⊕ (B/R)), 1-16eps))`. |
| `norm(v)` | Евклидова координатная норма. |
| `logMap(A,B)` | `R * (2/lambda_A) * atanh(min(norm(delta),1-eps)) * delta/norm(delta)`. |
| `expMap(A,v)` | `R * ((A/R) ⊕ ((v/R)/norm(v/R) * tanh(lambda_A * norm(v/R)/2)))`. |
| `tangentNorm(A,v)` | `lambda_A * norm(v) / R`. |
| `normalizePoint(p,FS)` | Clamp внутрь диска `R*(1-1e-15)`. |
| `volume(FS)` | `infinity`. |
| `areGeodesicSegmentsCrossing` | Переход Poincare -> Klein: `2u/(1+norm(u)^2)`. |

## FaR

```text
k = C * (vol / n)^(1 / dim), если vol >= 0
k = 1, если vol < 0
```

Для default border policy:

```text
vol = -1
k = 1
```

Силы:

```text
f_a(x) = x^2 / k
f_r(x) = k^2 / x
```

Шаг вершины:

```text
step = (disp / ||disp||) * min(||disp||, T)
newPos = expMap(pos, step)
```

## PoincareProjection

```text
R = min(figSize[0], figSize[1]) / 2
x_0 = sqrt(1 + x_1^2 + x_2^2)
y_i = R * x_i / (x_0 + 1)
```

Только `H2 -> 2D`.

## KleinProjection

| Шаг | Формула/поведение |
|---|---|
| Lorentz -> Klein | `X_0 = sqrt(1 + ||x||^2)`, `k = x / X_0`. |
| Камера | `A ∈ R^(2×n)`, `A A^T = I_2`, `y = A k`. |
| `kleinOrthogonal` | Строки `A` — первые два базисных вектора. |
| `kleinBestView` | Базовая камера и `--cameraCandidates - 1` случайных камер через Gram–Schmidt. |
| Objective | `+minVertexDist`, `+minEdgeVertexDist`, `+minAngle`, `-edgeCrossings` после нормализации между кандидатами. |
| Выход | `R y`, `R = min(figSize)/2`, внутри диска Klein радиуса `R`. |

## Metrics angle

```text
u = space.logMap(center, a)
v = space.logMap(center, b)
uNorm = space.tangentNorm(center, u)
vNorm = space.tangentNorm(center, v)
sumNorm = space.tangentNorm(center, u + v)
scal = (sumNorm^2 - uNorm^2 - vNorm^2) / 2
angle = arccos(clamp(scal / (uNorm * vNorm), -1, 1))
```
