---
title: GraphDrawing Obsidian Index
tags: [graphdrawing, index]
source: ../../PROJECT_REPORT.md
---

# GraphDrawing: индекс базы знаний

Этот каталог - Obsidian-версия монолитного отчета [PROJECT_REPORT.md](../../PROJECT_REPORT.md). Содержимое разложено на небольшие связанные заметки, чтобы проект было удобно изучать по модулям.

## Быстрый маршрут чтения

1. [[01_Project_Map|Карта проекта]] - что делает проект, как устроен pipeline, как собирается.
2. [[02_CLI_Data_IO|CLI, данные и I/O]] - флаги, JSON-форматы, чтение и запись.
3. [[03_Core_Model|Core model]] - `Graph`, `Embedding`, initial placement, border policy.
4. [[04_Space_Interface|Интерфейс Space]] - общий контракт геометрий и общие методы.
5. [[05_EuclideanSpace|EuclideanSpace]] - евклидова геометрия.
6. [[06_HyperbolicSpace|HyperbolicSpace]] - гиперболоидная модель и лоренцевы формулы.
7. [[07_PoincareSpace|PoincareSpace]] - диск Пуанкаре как пространство рисования.
8. [[08_Layout_Algorithms|Алгоритмы раскладки]] - `random` и FaR через `logMap/expMap`.
9. [[09_Projections|Проекции]] - `identity`, `orthogonal`, `poincare`, `kleinOrthogonal`, `kleinBestView`.
10. [[10_Metrics|Метрики]] - что реально считается после проекции.
11. [[11_Python_Tools|Python tools]] - `gen.py`, `render.py`, `render3d.py`, `gd_experiments.py`.
12. [[12_Limitations_Roadmap|Ограничения и развитие]] - незавершенные места и рекомендации.
13. [[13_Formula_Tables|Проверочные таблицы формул]] - компактный справочник по методам пространств.

## Основная карта модулей

| Модуль | Заметка |
|---|---|
| `src/app` | [[01_Project_Map#Runtime pipeline|Runtime pipeline]] |
| `src/io` | [[02_CLI_Data_IO|CLI, данные и I/O]] |
| `src/core` | [[03_Core_Model|Core model]] |
| `src/spaces` | [[04_Space_Interface|Space interface]], [[05_EuclideanSpace]], [[06_HyperbolicSpace]], [[07_PoincareSpace]] |
| `src/layouts` | [[08_Layout_Algorithms|Алгоритмы раскладки]] |
| `src/projections` | [[09_Projections|Проекции]] |
| `src/metrics` | [[10_Metrics|Метрики]] |
| Python scripts | [[11_Python_Tools|Python tools]] |

## Математический маршрут

Если нужен именно математический слой, читать так:

1. [[04_Space_Interface|Space interface]]
2. [[05_EuclideanSpace|EuclideanSpace]]
3. [[06_HyperbolicSpace|HyperbolicSpace]]
4. [[07_PoincareSpace|PoincareSpace]]
5. [[08_Layout_Algorithms#Fruchterman-Reingold через геометрию Space|FaR через геометрию Space]]
6. [[09_Projections#PoincareProjection|PoincareProjection]]
7. [[10_Metrics#Angles|Angles]]
8. [[13_Formula_Tables|Проверочные таблицы формул]]

## Важные предупреждения

- `HyperbolicSpace` хранит пространственную часть гиперболоида, а полную лоренцеву точку достраивает через `lift`.
- `PoincareSpace` сейчас только 2D и используется как drawing space после проекции.
- `PoincareProjection` поддерживает только `H2 -> 2D`; для `Hn -> 2D` доступны Klein-проекции.
- `OrthogonalProjection` и `IdentityProjection` работают только для Euclidean space.
- `KleinSpace` — 2D drawing space с диском и прямыми геодезическими; его метрики описывают евклидову читаемость рисунка.
- Метрики частично евклидовы даже после Poincare projection: vertex distances считаются по координатам, а не через `space.dist`.
