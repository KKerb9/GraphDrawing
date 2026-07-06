---
title: Project Map
tags: [graphdrawing, architecture]
source: ../../PROJECT_REPORT.md
---

# Карта проекта

Навигация: [[00_Index|Индекс]] | далее [[02_CLI_Data_IO|CLI, данные и I/O]]

## Область действия

Эта заметка описывает текущее состояние проекта `GraphDrawing` по исходному коду. Главный принцип: фиксируется реализованное поведение, а не теоретически возможная версия.

Проект строит геометрическую раскладку графа:

1. Граф читается из JSON-датасета.
2. Для графа создается `Embedding`.
3. Начальная расстановка задает стартовые координаты.
4. Алгоритм раскладки меняет координаты.
5. Результат проецируется в пространство рисования.
6. По результату считаются метрики.
7. Итог сохраняется в JSON.
8. Python-рендерер строит PNG.

Главная архитектурная идея: layout не должен знать формулы конкретной геометрии. Алгоритм FaR работает через [[04_Space_Interface|Space]]: `dist`, `logMap`, `expMap`, `tangentNorm`.

## Структура репозитория

```text
.
├── CMakeLists.txt
├── Makefile
├── README.md
├── requirements.txt
├── gen.py
├── render.py
├── render3d.py
├── gd_experiments.py
├── samples/
├── out/
├── src/
│   ├── app/
│   ├── core/
│   ├── io/
│   ├── layouts/
│   ├── metrics/
│   ├── projections/
│   └── spaces/
├── far_test.ipynb
└── test.ipynb
```

| Путь | Назначение |
|---|---|
| `src/app/` | Точка входа C++ приложения. |
| `src/core/` | Базовые структуры: граф, вложение, ошибки, начальная расстановка, границы области. |
| `src/io/` | CLI-конфигурация, чтение графов из JSON, запись результата. |
| `src/layouts/` | Алгоритмы раскладки. |
| `src/spaces/` | Геометрические пространства и формулы геометрии. |
| `src/projections/` | Преобразование вычислительного вложения в пространство рисования. |
| `src/metrics/` | Метрики качества изображения. |
| `samples/` | Входной датасет и сгенерированные картинки графов. |
| `out/` | Результаты запусков. |
| `gen.py` | Генерация графов. |
| `render.py` | 2D-визуализация JSON-результата. |
| `render3d.py` | 3D-визуализация JSON-результата. |
| `gd_experiments.py` | Утилиты для ноутбуков и пакетных экспериментов. |

## Сборка

Основной путь через CMake:

```bash
cmake -S . -B build
cmake --build build
```

`CMakeLists.txt`:

- требует CMake `3.16`;
- создает executable `graph_drawing`;
- собирает все `src/*.cpp`;
- использует C++17;
- подключает `nlohmann_json::nlohmann_json`;
- включает `-Wall -Wextra -Wpedantic`;
- опционально включает ASan/UBSan через `GD_ENABLE_SANITIZERS`.

`Makefile` собирает старый бинарь `artist` через `clang++`, C++23 и санитайзеры. Его `run-test` сейчас устарел: там указан `--projection euclidean`, но текущие проекции называются `identity`, `orthogonal`, `poincare`.

Python-зависимости:

```text
matplotlib==3.10.8
networkx==3.6.1
pandas>=2.0.0
tqdm>=4.66.0
```

## Runtime pipeline

Точка входа: `src/app/artist.cpp`.

1. Печатается `START` в `stderr`.
2. `parseArgs(argc, argv)` строит `Config`.
3. `JsonGraphReader(cfg.datasetPath)` читает граф.
4. `createSpace(cfg.spaceName, cfg.dimension)` создает вычислительное пространство.
5. `createBorderPolicy(cfg.borderPolicyName, cfg.dimension, cfg.seed)` создает политику границ.
6. `createProjection(cfg.projectionName)` создает проекцию.
7. `Embedding emb(graph, cfg.dimension)` создает вложение.
8. `createInitialPlacementStrategy(cfg.initialPlacementName)` задает стартовые координаты.
9. `createLayoutAlgorithm(cfg.algoName)` создает layout.
10. `algo->computeLayout(emb, *space, *borderPolicy)` меняет координаты.
11. `proj->project(emb, *space, cfg.figSize, cfg.finalDimension)` строит результат рисования.
12. `computeMetrics(res.embedding, *res.space)` считает метрики в drawing space.
13. `writeEmbeddingJson(...)` сохраняет JSON.

```text
CLI
  -> Config
  -> JsonGraphReader
  -> Graph
  -> Space
  -> BorderPolicy
  -> Embedding
  -> InitialPlacement
  -> LayoutAlgorithm
  -> Projection
  -> Metrics
  -> output JSON
```

## Связанные заметки

- [[02_CLI_Data_IO|CLI, данные и I/O]]
- [[03_Core_Model|Core model]]
- [[04_Space_Interface|Space interface]]
- [[08_Layout_Algorithms|Алгоритмы раскладки]]
- [[09_Projections|Проекции]]

