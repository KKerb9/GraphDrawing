---
title: Metrics
tags: [graphdrawing, metrics]
source: ../../PROJECT_REPORT.md
---

# Метрики

Навигация: [[00_Index|Индекс]] | назад [[09_Projections|Проекции]] | далее [[11_Python_Tools|Python tools]]

Файлы:

- `src/metrics/Metrics.h`
- `src/metrics/Metrics.cpp`

Метрики считаются после проекции:

```cpp
Metrics metrics = computeMetrics(res.embedding, *res.space);
```

Для гиперболического запуска с `poincare` метрики считаются в [[07_PoincareSpace|PoincareSpace]], а не в исходном [[06_HyperbolicSpace|HyperbolicSpace]].

## Поля Metrics

| Поле | Смысл в текущей реализации |
|---|---|
| `volume` | Координатный объем bounding box по точкам. |
| `minVertexDist` | Минимальное евклидово расстояние между координатами вершин. |
| `maxVertexDist` | Максимальное евклидово расстояние между координатами вершин. |
| `avgVertexDist` | Среднее евклидово расстояние между парами вершин. |
| `edgeCrossings` | Число пересечений геодезических ребер, только для `dim == 2`. |
| `minAngle` | Минимальный угол между смежными ребрами, только для `dim == 2`. |
| `maxAngle` | Максимальный угол между смежными ребрами, только для `dim == 2`. |
| `density` | `n / volume`, если `volume > 0`, иначе `0`. |
| `imageScore` | В C++ writer всегда `0`. |

## Volume

Bounding box:

```text
mns_j = min_i p_i[j]
mxs_j = max_i p_i[j]
```

Объем:

```text
volume = product_j (mxs_j - mns_j)
```

Это не `space.volume`. Комментарий в коде отмечает, что метрика пока закостылена на простую евклидову площадь/объем.

## Vertex distances

Несмотря на `space.dist`, расстояния между вершинами считаются евклидово:

```text
d(a,b) = sqrt(sum_i (a_i - b_i)^2)
```

Далее:

```text
minVertexDist = min_{i<j} d(p_i,p_j)
maxVertexDist = max_{i<j} d(p_i,p_j)
avgVertexDist = (sum_{i<j} d(p_i,p_j)) / (n(n-1)/2)
```

Если `n < 2`, все три величины равны `0`.

## Density

```text
density = n / volume, если volume > 0
density = 0, иначе
```

Сравнение с нулем идет через `Comparator`.

## Edge crossings

Считаются только при:

```text
dim == 2
```

Для каждой пары ребер:

```text
space.areGeodesicSegmentsCrossing(a,b,c,d)
```

Поведение зависит от space:

- [[05_EuclideanSpace|EuclideanSpace]]: прямые отрезки;
- [[07_PoincareSpace|PoincareSpace]]: Poincare -> Klein, затем прямые;
- [[06_HyperbolicSpace|HyperbolicSpace]]: spatial hyperboloid -> Klein, затем прямые.

Общие endpoints не считаются пересечением.

## Angles

Только `dim == 2` и только пары ребер с общей вершиной.

Для центра `center` и соседних точек `a`, `b`:

```text
u = space.logMap(center, a)
v = space.logMap(center, b)
uNorm = space.tangentNorm(center, u)
vNorm = space.tangentNorm(center, v)
```

Если одна из норм нулевая, угол `0`.

Скалярное произведение восстанавливается через норму суммы:

```text
sumNorm = space.tangentNorm(center, u + v)
scal = (sumNorm^2 - uNorm^2 - vNorm^2) / 2
```

Угол:

```text
cosValue = clamp(scal / (uNorm * vNorm), -1, 1)
angle = arccos(cosValue)
```

Нюанс: если `dim == 2`, но нет пар смежных ребер, `minAngle` может остаться `1e18`.

