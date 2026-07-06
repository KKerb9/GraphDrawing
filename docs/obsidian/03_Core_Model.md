---
title: Core Model
tags: [graphdrawing, core]
source: ../../PROJECT_REPORT.md
---

# Core model

Навигация: [[00_Index|Индекс]] | назад [[02_CLI_Data_IO|CLI, данные и I/O]] | далее [[04_Space_Interface|Space interface]]

## Базовые типы

`src/core/template.h`:

```cpp
using ld = long double;
using ll = long long;
using Pt = std::vector<ld>;
```

`Pt` - универсальный тип координат и касательных векторов.

Покоординатные операции:

| Операция | Формула |
|---|---|
| `a - b` | `(a_i - b_i)` |
| `a + b` | `(a_i + b_i)` |
| `a / s` | `(a_i / s)` |
| `a * s` | `(a_i s)` |
| `a += b` | покоординатное сложение |
| `a -= b` | покоординатное вычитание |

Операции используют `assert(a.size() == b.size())`.

## Comparator

`Comparator` хранит `EPS` и задает сравнения с точностью.

| Метод | Условие |
|---|---|
| `sgn(a)` | `1`, если `a > EPS`; `-1`, если `a < -EPS`; иначе `0`. |
| `eq(a,b)` | `|a-b| <= EPS`. |
| `sml(a,b)` | `a + EPS < b`. |
| `grt(a,b)` | `a - EPS > b`. |
| `smleq(a,b)` | `a - EPS <= b`. |
| `grteq(a,b)` | `a + EPS >= b`. |

## Ошибки

Все проектные ошибки наследуются от:

```cpp
class GraphDrawingError : public std::runtime_error
```

Это позволяет `main` ловить проектные ошибки через один catch-блок.

## Graph

`Graph` хранит неориентированный граф:

```cpp
std::vector<std::vector<int32_t>> _matr;
std::vector<std::pair<int32_t, int32_t>> _edges;
int32_t _n;
int32_t _m;
```

`addEdge(u,v)`:

1. Проверяет `0 <= u,v < _n`.
2. Добавляет `v` в `_matr[u]`.
3. Добавляет `u` в `_matr[v]`.
4. Добавляет `(u,v)` в `_edges`.
5. Увеличивает `_m`.

Нюансы:

- дубликаты ребер не удаляются;
- self-loop не запрещен;
- `Graph()` default constructor не инициализирует `_n` и `_m`;
- в основном pipeline используется `Graph(n)`.

## Embedding

`Embedding` связывает граф и координаты:

```cpp
const Graph& _graph;
int32_t curDim;
std::vector<Pt> _coords;
```

Конструкторы:

| Конструктор | Поведение |
|---|---|
| `Embedding(graph)` | `_coords` размера `graph.vertexCount()`, `curDim = 0`. |
| `Embedding(graph, dim)` | `_coords` размера `graph.vertexCount()`, `curDim = dim`. |
| `Embedding(graph, startCoords)` | Копирует координаты, `curDim = startCoords[0].size()`. |

`setPos(v,pos)` проверяет:

- `pos.size() == curDim`;
- `v` в диапазоне.

`Embedding` хранит ссылку на `Graph`, поэтому граф должен жить дольше вложения.

Нюанс: `Embedding(graph,startCoords)` небезопасен для пустого `startCoords`, потому что обращается к `startCoords[0]`.

## Initial placement

Интерфейс:

```cpp
class InitialPlacementStrategy {
  virtual void computeInitial(Embedding& emb, const Space& space, const BorderPolicy& borderPolicy) const = 0;
};
```

Реализации:

| Стратегия | Поведение |
|---|---|
| `zero` | Всем вершинам ставит `(0, ..., 0)`. |
| `random` | Для каждой вершины вызывает `borderPolicy.randomPoint(space)`. |

`ZeroInitialPlacement` также выставляет `emb.curDim = space.dimension()`.

## Border policy

Интерфейс:

| Метод | Назначение |
|---|---|
| `force(space, point)` | Сила от границы. В текущем FaR не используется. |
| `normalizePoint(space, point)` | Вернуть точку в допустимую область. |
| `randomPoint(space)` | Случайная стартовая точка. |
| `domainVolume(space)` | Объем области для параметра `k` в FaR. |

Единственная доступная реализация - `DefaultBorderPolicy`.

`DefaultBorderPolicy`:

- хранит `std::mt19937 _rng`;
- `force` возвращает нулевой вектор;
- `normalizePoint` возвращает точку без изменений;
- `randomPoint` генерирует каждую координату из `[-10, 10]`;
- `domainVolume` возвращает `-1`.

Следствие: при default border policy FaR считает объем недоступным и берет `k = 1`.

## Связанные заметки

- [[08_Layout_Algorithms|Алгоритмы раскладки]]
- [[04_Space_Interface|Space interface]]
- [[02_CLI_Data_IO|CLI, данные и I/O]]

