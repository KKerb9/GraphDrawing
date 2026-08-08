---
title: CLI, Data and I/O
tags: [graphdrawing, cli, io, json]
source: ../../PROJECT_REPORT.md
---

# CLI, данные и I/O

Навигация: [[00_Index|Индекс]] | назад [[01_Project_Map|Карта проекта]] | далее [[03_Core_Model|Core model]]

## CLI флаги

`Config.cpp` поддерживает:

| Флаг | Значение | Default |
|---|---|---|
| `--graph <name>` | Имя графа в датасете. | `SmallGraph` |
| `--algo <name>` | `random`, `far`. | `random` |
| `--space <name>` | `euclidean`, `hyperbolic`, `spherical`. | `euclidean` |
| `--borderPolicy <name>` | `default`. | `default` |
| `--initial-placement <name>` | `zero`, `random`. | `zero` |
| `--projection <name>` | `identity`, `orthogonal`, `poincare`, `kleinOrthogonal`, `kleinBestView`. | Автовыбор |
| `--dim <n>` | Размерность вычислительного пространства. | `2` |
| `--2d` | Итоговая размерность `2`. | Да |
| `--3d` | Итоговая размерность `3`. | Нет |
| `--FS <...>` | Размер области результата. | `[100, ..., 100]` |
| `--seed <uint32>` | Seed генератора. | `steady_clock` |
| `--cameraCandidates <int>` | Число камер для `kleinBestView`, включая базовую ортогональную. | `1000` |
| `--dataset <path>` | Путь к датасету. | `samples/dataset.json` |
| `--output <path>` | Путь к результату. | `out/<graph>_<algo>.json` |
| `--help` | Справка. | - |

## Автовыбор проекции

Если `--projection` не передан:

1. Если `space == "hyperbolic"`, `dimension == 2`, `finalDimension == 2`, выбирается `poincare`.
2. Иначе если `dimension > finalDimension`, выбирается `orthogonal`.
3. Иначе выбирается `identity`.

Важно: `identity` и `orthogonal` сейчас принимают только Euclidean space. Поэтому `--space hyperbolic --dim 3 --2d` без явной проекции выберет `orthogonal` и упадет уже в [[09_Projections|проекциях]].

Для визуализации `H^n` в 2D Klein проекция задаётся явно:

```bash
printf '\n' | ./build/graph_drawing --graph SmallGraph --algo random --space hyperbolic --initial-placement random --dim 4 --2d --projection kleinBestView --cameraCandidates 1000 --FS 500,500 --seed 1543
```

`kleinBestView` использует `--seed` для детерминированной генерации камер.

## Формат `--FS`

`parseFigSize` читает один argv-токен после `--FS`. Внутри токена можно использовать запятые или пробелы.

Работает:

```bash
--FS 50,50
--FS "50 50"
```

Не работает:

```bash
--FS 50 50
```

В последнем случае второе `50` становится отдельным неизвестным аргументом.

## Интерактивные параметры через stdin

Порядок чтения stdin:

1. `createBorderPolicy(...)` читает строку для border policy.
2. Если `--algo far`, `createLayoutAlgorithm(...)` читает строку для FaR.

Для `default` border policy строка должна быть пустой.

Для `far` строка может быть пустой или содержать:

```text
--C <number> --I <integer>
```

| Параметр | Смысл | Default |
|---|---|---|
| `--C` | Множитель идеальной длины `k`. | `1.0` |
| `--I` | Число итераций. | `100` |

Пример FaR с default-строками:

```bash
printf '\n\n' | ./build/graph_drawing --graph LargeGraph --algo far --space hyperbolic --initial-placement random --dim 2 --2d --FS 500,500 --seed 1543
```

## Текущие CLI-несоответствия

- `spherical` есть в списке config, но `createSpace` его не создает.
- `poly` есть как ветка в `readBorderPolicyInteractiveParams`, но не входит в список допустимых policies и не создается фабрикой.
- `--FS` не принимает несколько отдельных argv-токенов.
- `Makefile run-test` использует устаревшую проекцию `euclidean`.

## Входной датасет

`samples/dataset.json` - массив объектов:

```json
{
  "name": "graph1",
  "description": "default undirected graph",
  "gen_algo": "nx.gnp_random_graph",
  "params": {"n": 50, "p": 0.1},
  "nodes": [0, 1, 2],
  "edges": [[0, 1], [1, 2]]
}
```

Для C++ reader обязательны:

- root JSON является массивом;
- нужный объект имеет строковое `"name"`;
- нужный объект имеет массив `"edges"`.

`"nodes"` необязателен, но если он есть, он участвует в вычислении `max_id`. Число вершин:

```text
n = max_id + 1
```

## Выходной JSON

`EmbeddingWriter` пишет:

```json
{
  "graph_name": "...",
  "algo": "...",
  "space": "...",
  "drawing_space": "...",
  "initial_placement": "...",
  "projection": "...",
  "seed": 1543,
  "dimension": 2,
  "fig_size": [500, 500],
  "nodes": [
    {"id": 0, "x": 1.0, "y": 2.0}
  ],
  "edges": [
    [0, 1]
  ],
  "metrics": {
    "volume": 0.0,
    "minVertexDist": 0.0,
    "maxVertexDist": 0.0,
    "avgVertexDist": 0.0,
    "edgeCrossings": 0,
    "minAngle": 0.0,
    "maxAngle": 0.0,
    "density": 0.0,
    "imageScore": 0
  }
}
```

Для 3D результата у вершин дополнительно пишется `"z"`.

`dimension` - размерность после проекции. `fig_size` копируется из `cfg.figSize`; в `render.py` он задает пределы осей, а для Poincare и Klein также радиус `min(fig_size)/2`.

## JsonGraphReader

`JsonGraphReader`:

1. Читает файл целиком.
2. Удаляет UTF-8 BOM, если он есть.
3. Парсит JSON через `nlohmann::json`.
4. Проверяет, что root - массив.
5. Ищет объект с нужным `"name"`.
6. Проверяет `"edges"`.
7. Вычисляет число вершин.
8. Создает `Graph(n)`.
9. Добавляет ребра.

Проверки ребер:

- ребро - массив длины `2`;
- оба конца - JSON integer;
- id неотрицателен и помещается в `int32_t`;
- endpoint меньше `n`.

## EmbeddingWriter

`writeEmbeddingJson`:

1. Создает директорию результата.
2. Открывает `cfg.outputPath`.
3. Записывает config-поля.
4. Записывает вершины.
5. Записывает ребра.
6. Записывает метрики.

Координаты пишутся с `std::fixed << std::setprecision(6)`.

Writer предполагает итоговую размерность минимум `2`, что согласуется с текущими `--2d` и `--3d`.
