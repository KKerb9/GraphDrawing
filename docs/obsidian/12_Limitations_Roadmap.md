---
title: Limitations and Roadmap
tags: [graphdrawing, limitations, roadmap]
source: ../../PROJECT_REPORT.md
---

# Ограничения и развитие

Навигация: [[00_Index|Индекс]] | назад [[11_Python_Tools|Python tools]] | далее [[13_Formula_Tables|Таблицы формул]]

## Ограничения пространств и проекций

1. `spherical` указан в config, но не реализован.
2. [[09_Projections#PoincareProjection|PoincareProjection]] поддерживает только `H2 -> 2D`.
3. [[07_PoincareSpace|PoincareSpace]] поддерживает только `dim = 2`.
4. `IdentityProjection` и `OrthogonalProjection` поддерживают только Euclidean space.
5. Не реализована общая визуализация `hyperbolic dim > 2` в 2D/3D.

## Ограничения metrics

1. `volume` считается как координатный bounding box, а не через `space.volume`.
2. Vertex distances считаются евклидово, а не через `space.dist`.
3. `edgeCrossings`, `minAngle`, `maxAngle` считаются только для `dim == 2`.
4. `imageScore` в C++ всегда `0`.
5. Если нет смежных пар ребер в 2D, `minAngle` может остаться `1e18`.

Подробнее: [[10_Metrics|Метрики]].

## Ограничения border policy

1. Доступна только `default`.
2. `default` не ограничивает область.
3. `default` возвращает `domainVolume = -1`.
4. `force` определен в интерфейсе, но текущий FaR его не использует.

Подробнее: [[03_Core_Model#Border policy|Border policy]].

## Ограничения CLI

1. `--FS 50 50` не работает, потому что `--FS` читает один argv-токен.
2. Параметры `far` и `borderPolicy` читаются из stdin, а не из обычного CLI.
3. `--space spherical` проходит список допустимых имен, но падает при создании пространства.
4. Ветка `poly` для border policy есть в парсере stdin, но недоступна как реальная policy.

Подробнее: [[02_CLI_Data_IO|CLI, данные и I/O]].

## Ограничения data/core

1. `Graph` не удаляет дубликаты ребер.
2. `Graph` не запрещает self-loop.
3. `Graph()` default constructor не инициализирует `_n` и `_m`.
4. `Embedding(graph,startCoords)` использует `startCoords[0]`, поэтому пустой `startCoords` небезопасен.

Подробнее: [[03_Core_Model|Core model]].

## Добавление нового пространства

Нужно реализовать новый класс в `src/spaces/`:

1. `name()`
2. `dimension()`
3. `dist`
4. `norm`
5. `logMap`
6. `expMap`
7. `tangentNorm`
8. `normalizePoint`
9. `volume`
10. `areGeodesicSegmentsCrossing`
11. `isValid`

Затем добавить его в:

- `createSpace`;
- список `SPACE_NAMES`;
- документацию CLI.

Принцип: геометрические формулы должны оставаться внутри `Space`, а не переезжать в layout или metrics.

## Добавление `hyperbolic dim > 2 -> 2D`

Минимальный математический путь:

```text
x_0 = sqrt(1 + sum_i x_i^2)
y_i = R x_i / (x_0 + 1)
```

Это дает Poincare ball. Но для 2D нужна отдельная стратегия снижения размерности:

- первые две координаты;
- PCA;
- camera/projection plane;
- геодезическая плоскость.

Проекцию лучше назвать явно:

- `poincare-first`;
- `poincare-pca`;
- `poincare-camera`.

## Улучшение metrics

Последовательный путь:

1. Для `minVertexDist`, `maxVertexDist`, `avgVertexDist` использовать `space.dist`.
2. Для `volume` решить, нужна ли координатная площадь картинки, объем домена или риманов объем.
3. Для углов оставить схему через `logMap` и `tangentNorm`.
4. Для 3D edge crossings отдельно определить постановку.

## Улучшение CLI

Возможные изменения:

```bash
--far-C 1.0 --far-I 100
```

для переноса FaR-параметров из stdin в CLI.

Также:

- разрешить `--FS 50 50`;
- убрать `spherical` до реализации;
- убрать или реализовать `poly`;
- сделать ошибки автовыбора проекции более ранними.

