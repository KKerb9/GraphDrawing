---
title: Layout Algorithms
tags: [graphdrawing, layout, far]
source: ../../PROJECT_REPORT.md
---

# Алгоритмы раскладки

Навигация: [[00_Index|Индекс]] | назад [[07_PoincareSpace|PoincareSpace]] | далее [[09_Projections|Проекции]]

## Интерфейс LayoutAlgorithm

```cpp
virtual void computeLayout(
    Embedding& emb,
    const Space& space,
    const BorderPolicy& borderPolicy
) const = 0;
```

Фабрика:

| Имя | Класс |
|---|---|
| `random` | `RandomLayout` |
| `far` | `FruchtermanAndReingoldLayout` |

## RandomLayout

1. Проверяет `emb.dimension() == space.dimension()`.
2. Для каждой вершины:

```text
pos(v) = borderPolicy.randomPoint(space)
```

При [[03_Core_Model#Border policy|DefaultBorderPolicy]] каждая координата равномерна на `[-10, 10]`.

## Fruchterman-Reingold через геометрию Space

`FruchtermanAndReingoldLayout` использует:

- `logMap` для направления вдоль геодезики;
- `tangentNorm` для длины касательного вектора;
- `expMap` для шага по многообразию.

Параметры:

| Поле | Смысл | Default |
|---|---|---|
| `ITERS` | Число итераций. | `100` |
| `C` | Множитель идеальной длины. | `1.0` |

`computeLayout` вызывает `computeLayoutTest`.

## Идеальная длина `k`

```text
n = emb.size()
dim = space.dimension()
vol = borderPolicy.domainVolume(space)
```

Если `vol` не отрицателен с учетом `Comparator(EPS=1e-12)`:

```text
k = C * (vol / n)^(1 / dim)
```

Если `vol < 0`:

```text
k = 1
```

Для `DefaultBorderPolicy`:

```text
domainVolume(space) = -1
k = 1
```

## Силы

```text
f_a(x) = x^2 / k
f_r(x) = k^2 / x
```

где `x` - расстояние/длина касательного вектора между вершинами.

## Температура

```text
T = k
```

После каждой итерации:

```text
if it * 4 <= ITERS:
    T = T * 0.85
else:
    T = T * 0.95
```

В коде есть TODO заменить температуру.

## Отталкивание

Для каждой пары `i < j`:

```text
v1 = space.logMap(pos_i, pos_j)
v2 = space.logMap(pos_j, pos_i)
v1N = space.tangentNorm(pos_i, v1)
v2N = space.tangentNorm(pos_j, v2)
```

Если нормы не нулевые:

```text
disp[i] -= (v1 / v1N) * f_r(v1N)
disp[j] -= (v2 / v2N) * f_r(v2N)
```

`v1` направлен от `i` к `j`, поэтому минус двигает `i` от `j`.

## Притяжение по ребрам

Для каждого ребра `(i,j)`:

```text
v1 = space.logMap(pos_i, pos_j)
v2 = space.logMap(pos_j, pos_i)
v1N = space.tangentNorm(pos_i, v1)
v2N = space.tangentNorm(pos_j, v2)
```

Если нормы не нулевые:

```text
disp[i] += (v1 / v1N) * f_a(v1N)
disp[j] += (v2 / v2N) * f_a(v2N)
```

## Обновление позиции

```text
dispN = space.tangentNorm(pos_i, disp[i])
```

Если `dispN == 0`, вершина не двигается.

Иначе:

```text
step = (disp[i] / dispN) * min(dispN, T)
newPos = space.expMap(pos_i, step)
newPos = borderPolicy.normalizePoint(space, newPos)
pos_i = newPos
```

При `DefaultBorderPolicy` нормализация ничего не меняет.

## Legacy метод

`computeLayoutEuclideanOld` не используется текущим pipeline.

Особенности:

- использует обычные разности координат;
- вручную ограничивает точку `figSize`;
- содержит `area = 0`, поэтому `k = C * sqrt(area / n)` становится `0`;
- не должен рассматриваться как активная реализация.

