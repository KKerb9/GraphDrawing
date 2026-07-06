---
title: Python Tools
tags: [graphdrawing, python, render]
source: ../../PROJECT_REPORT.md
---

# Python tools

Навигация: [[00_Index|Индекс]] | назад [[10_Metrics|Метрики]] | далее [[12_Limitations_Roadmap|Ограничения и развитие]]

## gen.py

Назначение: генерировать графы в `samples/dataset.json` и сохранять PNG-эскизы.

| Функция | Назначение |
|---|---|
| `load_json()` | Загружает `samples/dataset.json` в глобальный `data`. |
| `upload_json()` | Перезаписывает `samples/dataset.json`. |
| `gen1(name,n,p)` | Генерирует `nx.gnp_random_graph(n,p)`. |
| `gen2(name,szs,P)` | Генерирует stochastic block model. |

`gen1` и `gen2`:

1. Создают `networkx` граф.
2. Строят `spring_layout`.
3. Сохраняют PNG в `samples/<name>.png`.
4. Добавляют объект в `data`.

## render.py

Назначение: 2D-визуализация result JSON.

Основные шаги:

1. Читает result JSON.
2. Собирает `networkx.Graph`.
3. Читает позиции `x`, `y`.
4. Берет ребра из result JSON или восстанавливает из датасета.
5. Читает `fig_size` длины `2`.
6. Рисует граф.
7. Сохраняет PNG.

Для обычного пространства ребра рисуются как прямые через `nx.draw_networkx_edges`.

Для `drawing_space == "poincare"` ребра рисуются как геодезики диска Пуанкаре.

## Poincare geodesic в render.py

`poincare_geodesic(a,b,radius,samples=80)` строит дугу окружности, ортогональной границе диска.

Если:

```text
det = a_x b_y - a_y b_x
|det| <= 1e-12 * max(R^2, 1)
```

возвращается прямой отрезок.

Иначе центр окружности `C=(C_x,C_y)` находится из:

```text
||C - a||^2 = rho^2
||C - b||^2 = rho^2
||C||^2 = rho^2 + R^2
```

Линейные уравнения:

```text
C · a = (||a||^2 + R^2) / 2
C · b = (||b||^2 + R^2) / 2
```

В коде:

```text
aValue = (a_x^2 + a_y^2 + R^2) / 2
bValue = (b_x^2 + b_y^2 + R^2) / 2
D = a_x b_y - a_y b_x

C_x = (aValue * b_y - a_y * bValue) / D
C_y = (a_x * bValue - aValue * b_x) / D
rho = sqrt(max(C_x^2 + C_y^2 - R^2, 0))
```

Дальше выбирается дуга внутри диска.

## fig_size в render.py

```text
xlim = [-fig_size[0]/2, fig_size[0]/2]
ylim = [-fig_size[1]/2, fig_size[1]/2]
```

Для Poincare:

```text
radius = min(fig_size) / 2
```

Размер matplotlib-фигуры фиксирован:

```python
plt.subplots(figsize=(8, 8))
fig.savefig(..., dpi=180)
```

`fig_size` влияет на координатный масштаб, а не напрямую на количество пикселей.

## render3d.py

Назначение: 3D-визуализация result JSON с `x`, `y`, `z`.

Особенности:

- требует `id`, `x`, `y`, `z`;
- ребра рисуются 3D-линиями;
- вершины через `ax.scatter`;
- подписи через `ax.text`;
- `fig_size` длины `3` задает `xlim`, `ylim`, `zlim`;
- `ax.set_box_aspect(fig_size)` сохраняет пропорции.

## gd_experiments.py

Утилиты для notebooks и серий экспериментов.

| Функция | Назначение |
|---|---|
| `setupPlotStyle()` | Настройка matplotlib. |
| `compileGd(repo)` | CMake configure/build. |
| `runGd(argv, repo, ...)` | Запуск `build/graph_drawing` с stdin. |
| `runRender(repo,jsonPath,...)` | Запуск `render.py`. |
| `setImageScore(res, score)` | Ручная запись `imageScore` в JSON. |
| `compareResults(...)` | Табличное сравнение запусков. |
| `plotMetricComparison(...)` | Bar plot метрик. |
| `bestOfN(...)` | Серия запусков с scoring по весам. |

`bestOfN`:

1. Делает `n` запусков.
2. Может менять seed как `baseSeed + i`.
3. Сохраняет результаты с `_batch_i`.
4. Собирает метрики.
5. Нормирует вклад выбранных метрик.
6. Возвращает лучший запуск, scores и все runs.

## Notebooks

В репозитории есть:

- `test.ipynb`;
- `far_test.ipynb`.

Основная переиспользуемая логика вынесена в `gd_experiments.py`.

