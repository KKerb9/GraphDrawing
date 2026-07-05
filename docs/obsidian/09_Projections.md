---
title: Projections
tags: [graphdrawing, projections]
source: ../../PROJECT_REPORT.md
---

# Проекции

Навигация: [[00_Index|Индекс]] | назад [[08_Layout_Algorithms|Алгоритмы раскладки]] | далее [[10_Metrics|Метрики]]

## Интерфейс Projection

```cpp
virtual ProjectionResult project(
    const Embedding& emb,
    const Space& space,
    const std::vector<int32_t>& figSize,
    int32_t finalDim
) const = 0;
```

`ProjectionResult`:

```cpp
Embedding embedding;
SpacePtr space;
```

Проекция возвращает координаты и новое пространство, в котором эти координаты интерпретируются.

## fitToFigSize

Проверки:

- `finalDim > 0`;
- `figSize.size() == finalDim`;
- стороны `figSize` положительные;
- координаты finite;
- размерность каждой точки равна `finalDim`.

Bounding box:

```text
mns_i = min_p p_i
mxs_i = max_p p_i
len_i = mxs_i - mns_i
```

Единый масштаб:

```text
scale = min_i figSize_i / len_i
```

по всем осям, где `len_i > 0`. Если все `len_i == 0`, то `scale = 1`.

Центрирование и масштаб:

```text
res_i = (p_i - (mns_i + mxs_i) / 2) * scale
```

Итог помещается в:

```text
[-figSize_i/2, figSize_i/2]
```

## IdentityProjection

Работает только для:

```text
space.name() == "euclidean"
finalDim == emb.dimension()
```

Действие:

1. Берет исходные координаты.
2. Вызывает `fitToFigSize`.
3. Возвращает `EuclideanSpace(finalDim)`.

## OrthogonalProjection

Работает только для:

```text
space.name() == "euclidean"
finalDim <= emb.dimension()
```

Берет первые `finalDim` координат:

```text
res = (coord_1, ..., coord_finalDim)
```

Затем масштабирует через `fitToFigSize` и возвращает `EuclideanSpace(finalDim)`.

## PoincareProjection

Работает только для:

```text
space.name() == "hyperbolic"
emb.dimension() == 2
finalDim == 2
```

При `emb.dimension() != 2` ошибка:

```text
PoincareProjection::project: only H2 to 2D projection is supported
```

Радиус:

```text
R = min(figSize[0], figSize[1]) / 2
```

Для гиперболической точки в spatial coordinates:

```text
x = (x_1, x_2)
x_0 = sqrt(1 + x_1^2 + x_2^2)
```

Координаты диска Пуанкаре:

```text
y_1 = R * x_1 / (x_0 + 1)
y_2 = R * x_2 / (x_0 + 1)
```

Общая формула:

```text
y = R * x_spatial / (x_0 + 1)
```

Выходное пространство:

```text
PoincareSpace(2, R)
```

Если `figSize = [500, 300]`, радиус равен `150`, потому что используется минимум сторон.

## Поддержка проекций

| Проекция | Вход | Выход | Ограничение |
|---|---|---|---|
| `identity` | Euclidean | Euclidean | `finalDim == emb.dimension()` |
| `orthogonal` | Euclidean | Euclidean | `finalDim <= emb.dimension()` |
| `poincare` | Hyperbolic | Poincare | Только `H2 -> 2D` |

Сейчас не реализованы:

- `hyperbolic dim=3 -> --2d`;
- `hyperbolic dim=4 -> --2d`;
- `hyperbolic dim=3 -> --3d` через Poincare ball;
- общая проекция неевклидовых пространств.

