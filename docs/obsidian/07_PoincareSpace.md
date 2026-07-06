---
title: PoincareSpace
tags: [graphdrawing, math, spaces, poincare]
source: ../../PROJECT_REPORT.md
---

# PoincareSpace

Навигация: [[00_Index|Индекс]] | назад [[06_HyperbolicSpace|HyperbolicSpace]] | далее [[08_Layout_Algorithms|Алгоритмы раскладки]]

Файлы:

- `src/spaces/PoincareSpace.h`
- `src/spaces/PoincareSpace.cpp`

## Назначение

`PoincareSpace` - пространство рисования после [[09_Projections#PoincareProjection|PoincareProjection]]. Оно не создается через `createSpace` как вычислительное пространство.

Конструктор разрешает только:

```text
dim = 2
radius > 0
radius finite
```

## Модель координат

Точка хранится в диске радиуса `R`:

```text
p = (p_x, p_y)
||p|| < R
```

Для формул используется единичный диск:

```text
x = p / R
p = R x
```

`isValid(p)` проверяет:

- размерность `2`;
- finite координаты;
- `||p|| < R`.

## Евклидова норма

```text
euclideanNorm(p) = sqrt(p_x^2 + p_y^2)
```

`norm(vec)` делает то же самое с проверками. Это не риманова норма в точке; для нее есть `tangentNorm`.

## Сложение Мебиуса

Работает в единичном диске.

```text
aSq = ||a||^2
bSq = ||b||^2
dot = a · b
den = 1 + 2 dot + aSq bSq
```

Если `den <= 0`, выбрасывается ошибка.

```text
aCoeff = 1 + 2 dot + bSq
bCoeff = 1 - aSq
a ⊕ b = (aCoeff * a + bCoeff * b) / den
```

Координатно:

```text
(a ⊕ b)_i = ((1 + 2<a,b> + ||b||^2) a_i + (1 - ||a||^2) b_i)
            / (1 + 2<a,b> + ||a||^2 ||b||^2)
```

## Расстояние

```text
x = A / R
y = B / R
delta = (-x) ⊕ y
deltaNorm = min(||delta||, 1 - 16 epsilon)
dist(A,B) = 2 atanh(deltaNorm)
```

`R` - масштаб координат изображения; геометрическая формула работает после деления на `R`.

## logMap

```text
x = A / R
y = B / R
delta = (-x) ⊕ y
deltaNorm = ||delta||
```

Если `deltaNorm < 16 * epsilon`, возвращается нулевой вектор.

```text
lambda = 2 / (1 - ||x||^2)
safeNorm = min(deltaNorm, 1 - EPS)
coeff = (2 / lambda) * atanh(safeNorm) / deltaNorm
return R * coeff * delta
```

## expMap

```text
x = A / R
v = tangent / R
vNorm = ||v||
```

Если `vNorm < 16 * epsilon`, возвращается `from`.

```text
lambda = 2 / (1 - ||x||^2)
step = (v / vNorm) * tanh(lambda * vNorm / 2)
result = R * (x ⊕ step)
```

Затем `result` нормализуется внутрь диска.

## tangentNorm

```text
x = at / R
lambda = 2 / (1 - ||x||^2)
tangentNorm(at, tangent) = lambda * ||tangent|| / R
```

## normalizePoint

```text
pointNorm = ||p||
maxNorm = R * (1 - 1e-15)
```

Если `pointNorm < maxNorm`, возвращается `p`. Иначе:

```text
return p / pointNorm * maxNorm
```

`figSize` используется только для проверки длины, не для вычисления радиуса.

## volume

`volume(figSize)` проверяет длину `figSize` и возвращает:

```text
infinity
```

## Пересечения геодезических

Переход Poincare -> Klein:

```text
u = p / R
K(u) = 2u / (1 + ||u||^2)
```

Затем используется евклидова проверка отрезков:

```text
areEuclideanSegmentsCrossing2D(K(a), K(b), K(c), K(d))
```

## Краткая таблица

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
