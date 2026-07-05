---
title: Space Interface
tags: [graphdrawing, math, spaces]
source: ../../PROJECT_REPORT.md
---

# Интерфейс Space

Навигация: [[00_Index|Индекс]] | назад [[03_Core_Model|Core model]] | далее [[05_EuclideanSpace|EuclideanSpace]]

## Назначение

`Space` задает геометрию, в которой лежит вычислительное вложение. Layout должен работать через этот интерфейс и не дублировать геометрические формулы.

## Контракт Space

| Метод | Смысл |
|---|---|
| `name()` | Имя пространства. |
| `dimension()` | Размерность координат `Pt`. |
| `dist(a,b)` | Расстояние между точками. |
| `norm(vec)` | Норма обычного координатного вектора. |
| `logMap(from,to)` | Точка `to` как касательный вектор в `from`. |
| `expMap(from,tangent)` | Шаг из `from` по касательному вектору. |
| `tangentNorm(at,tangent)` | Риманова норма касательного вектора в точке `at`. |
| `normalizePoint(p,figSize)` | Ограничение точки областью размера `figSize`. |
| `volume(figSize)` | Объем области размера `figSize`. |
| `areGeodesicSegmentsCrossing(a,b,c,d)` | Пересечение геодезических отрезков. |
| `isValid(c)` | Валидность координаты как точки пространства. |

## determinantBareiss

`Space::determinantBareiss` вычисляет детерминант квадратной матрицы методом Барейса.

Обновление на шаге:

```text
a[i][j] = (a[i][j] * a[k][k] - a[i][k] * a[k][j]) / prev
```

где `prev` - предыдущий pivot, на первом шаге `prev = 1`.

Поведение:

- `n = 0` дает `1`;
- матрица должна быть квадратной;
- pivot выбирается по максимальному `abs`;
- при перестановке строк меняется знак;
- если pivot меньше `1e-18`, возвращается `0`.

Метод используется для объемов через матрицу Грама.

## areEuclideanSegmentsCrossing2D

Общая проверка пересечения двух 2D-отрезков.

Для векторов:

```text
cross(u,v) = u_x v_y - u_y v_x
dot(u,v) = u_x v_x + u_y v_y
```

Точка `p` лежит на отрезке `ab`, если:

```text
cross(b-a, p-a) = 0
dot(p-a, b-a) >= 0
dot(a-b, p-b) >= 0
```

Правила:

- если отрезки имеют общий endpoint, возвращается `false`;
- если точка лежит на другом отрезке, возвращается `true`;
- иначе используются знаки ориентированных площадей.

## Реализованные пространства

| Пространство | Назначение |
|---|---|
| [[05_EuclideanSpace|EuclideanSpace]] | Вычислительное и drawing space для евклидовых результатов. |
| [[06_HyperbolicSpace|HyperbolicSpace]] | Вычислительное гиперболическое пространство в модели гиперболоида. |
| [[07_PoincareSpace|PoincareSpace]] | Drawing space после `PoincareProjection`, только 2D. |

