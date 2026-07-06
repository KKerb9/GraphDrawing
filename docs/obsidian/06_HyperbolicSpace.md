---
title: HyperbolicSpace
tags: [graphdrawing, math, spaces, hyperbolic]
source: ../../PROJECT_REPORT.md
---

# HyperbolicSpace

Навигация: [[00_Index|Индекс]] | назад [[05_EuclideanSpace|EuclideanSpace]] | далее [[07_PoincareSpace|PoincareSpace]]

Файлы:

- `src/spaces/HyperbolicSpace.h`
- `src/spaces/HyperbolicSpace.cpp`

## Модель координат

`HyperbolicSpace` хранит не полный лоренцев вектор, а пространственную часть гиперболоида:

```text
x = (x_1, ..., x_d) in R^d
```

Полная точка строится методом `lift`:

```text
X = lift(x) = (x_0, x_1, ..., x_d)
x_0 = sqrt(1 + sum_i x_i^2)
```

Тогда:

```text
-x_0^2 + x_1^2 + ... + x_d^2 = -1
x_0 > 0
```

`isValid(x)` проверяет:

- `size(x) == d`;
- все координаты finite.

Любой конечный `x in R^d` валиден, потому что `lift(x)` всегда лежит на гиперболоиде.

## Лоренцево произведение

Для:

```text
A = (A_0, A_1, ..., A_d)
B = (B_0, B_1, ..., B_d)
```

реализовано:

```text
<A,B>_L = -A_0 B_0 + sum_{i=1}^{d} A_i B_i
```

Метод `lorentzProd` требует размерность `d + 1`.

## Касательные векторы

Касательный вектор в коде хранится как `Pt` длины `d`:

```text
v = (v_1, ..., v_d)
```

Полный лоренцев касательный вектор в точке `x`:

```text
X = lift(x)
X_0 = sqrt(1 + ||x||^2)
V_0 = (x · v) / X_0
V_i = v_i
V = (V_0, v_1, ..., v_d)
```

Проверка касательности:

```text
<X,V>_L = -X_0 V_0 + x · v
         = -X_0 * (x · v / X_0) + x · v
         = 0
```

`defaultToTangent(W)` отбрасывает нулевую координату:

```text
defaultToTangent(W) = (W_1, ..., W_d)
```

## Расстояние

Для `a,b in R^d`:

```text
A = lift(a)
B = lift(b)
alpha = -<A,B>_L
```

В координатах:

```text
alpha = sqrt(1 + ||a||^2) sqrt(1 + ||b||^2) - a · b
```

Код делает clamp:

```text
alpha = max(alpha, 1)
```

и возвращает:

```text
dist(a,b) = arcosh(alpha)
```

## norm и tangentNorm

`norm(v)` - евклидова координатная норма:

```text
norm(v) = sqrt(sum_i v_i^2)
```

Риманова норма:

```text
V = tangentToDefault(x, v)
tangentNorm(x, v) = sqrt(max(<V,V>_L, 0))
```

В развернутом виде:

```text
tangentNorm(x,v)^2 = ||v||^2 - (x · v)^2 / (1 + ||x||^2)
```

## logMap

```text
X = lift(from)
Y = lift(to)
alpha = -<X,Y>_L
alpha = max(alpha, 1)
d = arcosh(alpha)
```

Если `d < 1e-15`, возвращается нулевой вектор.

Иначе:

```text
U = Y - alpha X
uNormSq = <U,U>_L
uNorm = sqrt(uNormSq)
coeff = d / uNorm
U = coeff * U
return defaultToTangent(U)
```

Эквивалентная стандартная формула:

```text
log_X(Y) = d * (Y - cosh(d) X) / sinh(d)
```

## expMap

```text
X = lift(from)
V = tangentToDefault(from, tangent)
vNormSq = <V,V>_L
vNorm = sqrt(max(vNormSq, 0))
```

Если `vNorm < 1e-15`, возвращается `from`.

Иначе:

```text
Z = cosh(vNorm) X + (sinh(vNorm) / vNorm) V
return defaultToTangent(Z)
```

## normalizePoint

Ограничение гиперболическим шаром вокруг начала:

```text
radius = min(figSize) / 2
center = (0, ..., 0)
```

Если `dist(center,p) <= radius`, точка возвращается без изменений.

Иначе:

```text
v = logMap(center, p)
vN = tangentNorm(center, v)
return expMap(center, v / vN * radius)
```

Важно: [[03_Core_Model#Border policy|DefaultBorderPolicy]] не вызывает `space.normalizePoint`, поэтому в обычном запуске эта нормализация не применяется.

## volume

Строятся касательные базисные векторы в origin:

```text
e_i = (0, ..., figSize_i, ..., 0)
```

Матрица Грама:

```text
G_ij = (||e_i + e_j||^2 - ||e_i||^2 - ||e_j||^2) / 2
```

Возвращается:

```text
sqrt(det(G))
```

Так как точка - origin, это фактически объем касательного параллелепипеда, а не интегральный гиперболический объем области.

## Пересечения геодезических

Только `d = 2`.

Переход в модель Клейна:

```text
x_0 = sqrt(1 + p_1^2 + p_2^2)
K(p) = p / x_0
```

В Клейне геодезики - евклидовы хорды:

```text
areGeodesicSegmentsCrossing(a,b,c,d)
  = areEuclideanSegmentsCrossing2D(K(a), K(b), K(c), K(d))
```

## Краткая таблица

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
