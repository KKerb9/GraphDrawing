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

Для `drawing_space == "klein"` рендерер рисует границу диска и оставляет
рёбра прямыми отрезками.

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

Интерфейс пакетных экспериментов на `pandas.DataFrame`. Каждый запуск — одна
строка с метриками, путями, выводом процесса и maps применённых флагов. JSON
с координатами и рёбрами остаётся отдельным файлом по `jsonPath`.

| Функция | Назначение |
|---|---|
| `setupPlotStyle()` | Применяет общую тёмную Seaborn-тему. |
| `compileGd(repo)` | CMake configure/build. |
| `runTest(repo, flags, ...)` | Один запуск и однострочный DataFrame. |
| `runTests(repo, flags, n, ...)` | Серия из `n` запусков без агрегации метрик. |
| `scoreRuns(df, weights)` | Добавляет score по map весов в копию DataFrame. |
| `bestRun(df)` | Возвращает строку с максимальным score как DataFrame. |
| `setImageScore(df, run, score)` | Меняет ручную оценку только в DataFrame. |
| `renderRun(repo, df, run, ...)` | Рендерит JSON запуска и возвращает `Path` PNG. |
| `displayTwo(image1, image2, ...)` | Показывает рядом два PNG графов для визуального сравнения. |
| `plotMetrics(df, metric, title=..., info=...)` | Line plot одной метрики либо сетка всех метрик с заголовком и блоком подписи. |

Флаги запуска передаются упорядоченными списками CLI-токенов:

```python
flags = ["--graph", "graph1", "--algo", "far", "--3d"]
algoFlags = ["--C", "1.0", "--I", "100"]
weights = {"volume": -1.0, "minVertexDist": 1.0}
```

Список передаётся бинарю в том же порядке. `flags`, `algoFlags` и `borderFlags`
могут переиспользоваться и изменяться внутри generator: `runTests` копирует
очередной список до запуска. Для совместимости также принимается dict, где
порядок ключей задаёт порядок аргументов, а `None` передаёт флаг без значения.

Callback `*ForRun` получает `range(n)` и выдаёт список флагов для каждого
запуска:

```python
def flagsForRun(indices):
    for index in indices:
        flags[1] = graphs[index]
        yield flags
```

Типовой поток:

```python
runs = runTests(repo, flags, 20, outDir=tempDir, algoFlags=algoFlags)
scoredRuns = scoreRuns(runs, weights)
best = bestRun(scoredRuns)
plotMetrics(
    scoredRuns,
    "all",
    title="baseTreePath",
    info={"Граф": "baseTreePath", "Проекция": "kleinOrthogonal"},
)
```

`plotMetrics` использует номер запуска по X и исходное значение метрики по Y;
Seaborn вызывается без усреднения. `runTests` создаёт JSON с `_batch_i`.
Если все строки имеют ненулевой `returnCode`, функция не рисует пустые оси, а
показывает последние сообщения `stderr`: сначала нужно исправить запуск.

Для сравнения отрендеренных запусков используйте `displayTwo`. Она принимает
пути к PNG (или созданные из файлов `IPython.display.Image`), отдельные
подписи и общий заголовок:

```python
displayTwo(
    firstPng,
    secondPng,
    title1="kleinOrthogonal",
    title2="kleinBestView",
    title="baseTreePath",
)
```

Артефакты проверок следует создавать во временном каталоге вне репозитория и
удалять после окончания работы. Пользовательские результаты `out/` не удаляются.

## Notebooks

В репозитории есть:

- `test.ipynb`;
- `far_test.ipynb`.

Существующие notebooks остаются историческими артефактами и не обновляются под
новый интерфейс. Основная переиспользуемая логика вынесена в `gd_experiments.py`.
