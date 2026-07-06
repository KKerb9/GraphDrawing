---
title: EuclideanSpace
tags: [graphdrawing, math, spaces, euclidean]
source: ../../PROJECT_REPORT.md
---

# EuclideanSpace

Навигация: [[00_Index|Индекс]] | назад [[04_Space_Interface|Space interface]] | далее [[06_HyperbolicSpace|HyperbolicSpace]]

Файлы:

- `src/spaces/EuclideanSpace.h`
- `src/spaces/EuclideanSpace.cpp`

## Модель координат

Точка:

```text
x = (x_1, ..., x_d) in R^d
```

`isValid(x)` проверяет только:

```text
size(x) == d
```

Проверки на `finite` сейчас нет.

## Расстояние и норма

```text
dist(a,b) = sqrt(sum_i (a_i - b_i)^2)
norm(v) = sqrt(sum_i v_i^2)
```

## Log и exp

В Euclidean space касательное пространство в любой точке отождествлено с `R^d`.

```text
log_from(to) = to - from
exp_from(v) = from + v
tangentNorm(at, v) = norm(v)
```

`at` проверяется на размерность, но в формуле `tangentNorm` не участвует.

## normalizePoint

Покоординатный clamp в параллелепипед:

```text
res_i = min(figSize_i / 2, max(-figSize_i / 2, p_i))
```

Область:

```text
[-figSize_1/2, figSize_1/2] x ... x [-figSize_d/2, figSize_d/2]
```

## volume

Матрица Грама:

```text
G_ij = figSize_i^2, если i = j
G_ij = 0, если i != j
```

Возвращается:

```text
volume = sqrt(det(G))
```

Для положительных `figSize_i`:

```text
volume = product_i figSize_i
```

## areGeodesicSegmentsCrossing

Поддерживается только `d = 2`.

Геодезики - прямые отрезки, поэтому используется [[04_Space_Interface#areEuclideanSegmentsCrossing2D|areEuclideanSegmentsCrossing2D]].

## Краткая таблица

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

