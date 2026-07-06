# GraphDrawing: технический отчет по архитектуре и математике

## 0. Область действия документа

Этот документ описывает текущее состояние проекта `GraphDrawing` по исходному коду в репозитории. Основной исполняемый файл строится из C++ модулей в `src/`, а Python-скрипты используются для генерации данных, визуализации и экспериментов.

Главный принцип отчета: описывается именно реализованное поведение. Если в математике существует более общий или более стандартный вариант, но в коде реализована упрощенная версия, в отчете фиксируется реализация.

## 1. Назначение проекта

Проект решает задачу построения геометрической раскладки графа:

1. Граф читается из JSON-датасета.
2. Для графа создается вложение `Embedding`: каждой вершине назначается точка в выбранном пространстве.
3. Начальная расстановка задается выбранной стратегией.
4. Алгоритм раскладки изменяет координаты вершин.
5. Результат проецируется в пространство рисования.
6. По результату считаются метрики.
7. Итог сохраняется в JSON.
8. Python-рендерер строит PNG по JSON.

Главная математическая идея проекта - отделить алгоритмы раскладки от конкретной геометрии. Алгоритм Fruchterman-Reingold работает через интерфейс `Space`: расстояния, логарифмическое отображение, экспоненциальное отображение и норму касательного вектора.

## 2. Структура репозитория

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

Краткое назначение директорий:

| Путь | Назначение |
|---|---|
| `src/app/` | Точка входа C++ приложения. |
| `src/core/` | Базовые структуры: граф, вложение, ошибки, начальная расстановка, границы области. |
| `src/io/` | CLI-конфигурация, чтение графов из JSON, запись результата. |
| `src/layouts/` | Алгоритмы раскладки. |
| `src/spaces/` | Геометрические пространства и все формулы геометрии. |
| `src/projections/` | Преобразование вычислительного вложения в пространство рисования. |
| `src/metrics/` | Метрики качества изображения. |
| `samples/` | Входной датасет и эталонные/сгенерированные картинки графов. |
| `out/` | Результаты запусков. |
| `gen.py` | Генерация графов в `samples/dataset.json`. |
| `render.py` | 2D-визуализация JSON-результата. |
| `render3d.py` | 3D-визуализация JSON-результата. |
| `gd_experiments.py` | Утилиты для ноутбуков и пакетных экспериментов. |

## 3. Сборка и зависимости

### 3.1. C++ сборка через CMake

Основной способ сборки:

```bash
cmake -S . -B build
cmake --build build
```

`CMakeLists.txt`:

- требует CMake `3.16`;
- создает executable `graph_drawing`;
- собирает все `src/*.cpp` рекурсивно;
- использует C++17;
- подключает `nlohmann_json::nlohmann_json`;
- включает предупреждения `-Wall -Wextra -Wpedantic`;
- опционально включает ASan/UBSan через `GD_ENABLE_SANITIZERS`.

### 3.2. Makefile

`Makefile` собирает бинарь `artist` через `clang++`, C++23 и санитайзеры. Он выглядит как более старый путь сборки:

- цель `artist` собирает все `src/*.cpp`;
- цель `clear` удаляет `artist`;
- цель `run-test` запускает пример.

Важный нюанс: в `run-test` указан `--projection euclidean`, но в текущем `Config.cpp` допустимые имена проекций: `identity`, `orthogonal`, `poincare`. Поэтому этот пример в `Makefile` не соответствует текущему CLI.

### 3.3. Python зависимости

`requirements.txt`:

```text
matplotlib==3.10.8
networkx==3.6.1
pandas>=2.0.0
tqdm>=4.66.0
```

Python используется не в C++ pipeline, а для генерации графов, рендера результатов и ноутбучных экспериментов.

## 4. Основной runtime pipeline

Точка входа: `src/app/artist.cpp`.

Порядок выполнения:

1. Печатается `START` в `stderr`.
2. `parseArgs(argc, argv)` строит `Config`.
3. `JsonGraphReader(cfg.datasetPath)` читает граф по имени `cfg.graphName`.
4. `createSpace(cfg.spaceName, cfg.dimension)` создает вычислительное пространство.
5. `createBorderPolicy(cfg.borderPolicyName, cfg.dimension, cfg.seed)` создает политику границ.
6. `createProjection(cfg.projectionName)` создает проекцию.
7. `Embedding emb(graph, cfg.dimension)` создает вложение нужной размерности.
8. `createInitialPlacementStrategy(cfg.initialPlacementName)` задает начальные координаты.
9. `createLayoutAlgorithm(cfg.algoName)` создает алгоритм раскладки.
10. `algo->computeLayout(emb, *space, *borderPolicy)` изменяет координаты.
11. `proj->project(emb, *space, cfg.figSize, cfg.finalDimension)` строит результат для рисования.
12. `computeMetrics(res.embedding, *res.space)` считает метрики уже в пространстве рисования.
13. `writeEmbeddingJson(...)` сохраняет JSON.

Схематически:

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

## 5. CLI и конфигурация

### 5.1. Поддерживаемые флаги

`Config.cpp` поддерживает:

| Флаг | Значение | Значение по умолчанию |
|---|---|---|
| `--graph <name>` | Имя графа в датасете. | `SmallGraph` |
| `--algo <name>` | Алгоритм: `random`, `far`. | `random` |
| `--space <name>` | Пространство: `euclidean`, `hyperbolic`, `spherical`. | `euclidean` |
| `--borderPolicy <name>` | Политика границ: `default`. | `default` |
| `--initial-placement <name>` | Начальная расстановка: `zero`, `random`. | `zero` |
| `--projection <name>` | Проекция: `identity`, `orthogonal`, `poincare`. | Автовыбор |
| `--dim <n>` | Размерность вычислительного пространства. | `2` |
| `--2d` | Итоговая размерность результата равна `2`. | Да |
| `--3d` | Итоговая размерность результата равна `3`. | Нет |
| `--FS <...>` | Размер области результата. | `[100, ..., 100]` |
| `--seed <uint32>` | Seed генератора случайных чисел. | Время `steady_clock` |
| `--dataset <path>` | Путь к датасету. | `samples/dataset.json` |
| `--output <path>` | Путь к результату. | `out/<graph>_<algo>.json` |
| `--help` | Печать справки. | - |

### 5.2. Автовыбор проекции

Если пользователь не передал `--projection`, логика такая:

1. Если `space == "hyperbolic"`, `dimension == 2`, `finalDimension == 2`, выбирается `poincare`.
2. Иначе если `dimension > finalDimension`, выбирается `orthogonal`.
3. Иначе выбирается `identity`.

Важно: `orthogonal` и `identity` сейчас реализованы только для `euclidean`. Поэтому, например, `--space hyperbolic --dim 3 --2d` без явной проекции автоматически выберет `orthogonal`, а затем упадет в `OrthogonalProjection`, потому что она принимает только Euclidean space.

### 5.3. Формат `--FS`

`parseFigSize` читает ровно один argv-токен после `--FS`. Внутри этого токена допускаются числа через запятую или пробел.

Рабочие варианты:

```bash
--FS 50,50
--FS "50 50"
```

Нерабочий вариант:

```bash
--FS 50 50
```

Во втором случае CLI видит первое `50` как значение `--FS`, а второе `50` как отдельный неизвестный аргумент.

### 5.4. Интерактивные параметры

После парсинга CLI некоторые компоненты читают параметры из `stdin`.

Порядок чтения:

1. `createBorderPolicy(...)` читает строку для border policy.
2. Если `--algo far`, `createLayoutAlgorithm(...)` читает строку для FaR.

Для `default` border policy ожидается пустая строка. Любая непустая строка приводит к ошибке `BorderPolicy default: no params expected`.

Для `far` строка может быть пустой или содержать пары:

```text
--C <number> --I <integer>
```

Параметры:

| Параметр | Смысл | Default |
|---|---|---|
| `--C` | Множитель для идеальной длины `k`. | `1.0` |
| `--I` | Число итераций. | `100` |

Пример запуска с пустыми интерактивными строками:

```bash
printf '\n\n' | ./build/graph_drawing --graph LargeGraph --algo far --space hyperbolic --initial-placement random --dim 2 --2d --FS 500,500 --seed 1543
```

Первая пустая строка относится к `BorderPolicy default`, вторая - к `far`.

### 5.5. Текущие несоответствия в CLI

В `SPACE_NAMES` есть `spherical`, но `createSpace` поддерживает только:

- `euclidean`;
- `hyperbolic`.

Поэтому `--space spherical` пройдет первичную проверку `Config::isValid`, но затем упадет в `createSpace`.

В `readBorderPolicyInteractiveParams` есть ветка `poly`, но:

- `BORDER_POLICY_NAMES` содержит только `default`;
- `createBorderPolicy` создает только `default`.

То есть `poly` сейчас не является доступной политикой.

## 6. Форматы данных

### 6.1. Входной датасет

Файл `samples/dataset.json` - массив объектов. Каждый объект описывает граф:

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

- корень JSON должен быть массивом;
- нужный элемент должен иметь строковое поле `"name"`;
- нужный элемент должен иметь массив `"edges"`.

Поле `"nodes"` необязательно, но если есть, оно используется для определения максимального id вершины. Итоговое число вершин равно `max_id + 1`, где `max_id` берется по всем вершинам из `"nodes"` и концам ребер.

### 6.2. Выходной JSON

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

Для 3D результата `nodes` дополнительно содержит `"z"`.

Поле `"dimension"` - это размерность после проекции, то есть размерность `res.embedding`, а не обязательно вычислительная `cfg.dimension`.

Поле `"fig_size"` копируется из `cfg.figSize`. Для `render.py` оно задает координатные пределы осей. Для Poincare-рендера радиус диска равен `min(fig_size) / 2`.

## 7. Базовые типы и core-модуль

### 7.1. `template.h`

Основные псевдонимы:

```cpp
using ld = long double;
using ll = long long;
using Pt = std::vector<ld>;
```

`Pt` - универсальный тип координат и касательных векторов.

Перегружены операции:

| Операция | Формула |
|---|---|
| `a - b` | `(a_i - b_i)` |
| `a + b` | `(a_i + b_i)` |
| `a / s` | `(a_i / s)` |
| `a * s` | `(a_i s)` |
| `a += b` | покоординатное сложение |
| `a -= b` | покоординатное вычитание |

Все эти операции используют `assert(a.size() == b.size())`, то есть проверка размерности выполняется только в debug/assert-enabled режиме.

`Comparator` инкапсулирует сравнение с точностью `EPS`:

| Метод | Условие |
|---|---|
| `sgn(a)` | `1`, если `a > EPS`; `-1`, если `a < -EPS`; иначе `0`. |
| `eq(a,b)` | `|a-b| <= EPS`. |
| `sml(a,b)` | `a + EPS < b`. |
| `grt(a,b)` | `a - EPS > b`. |
| `smleq(a,b)` | `a - EPS <= b`. |
| `grteq(a,b)` | `a + EPS >= b`. |

### 7.2. `Errors.h`

Все доменные ошибки наследуются от:

```cpp
class GraphDrawingError : public std::runtime_error
```

Это позволяет `main` ловить проектные ошибки одним catch-блоком.

### 7.3. `Graph`

`Graph` хранит неориентированный граф:

- `_matr`: список соседей;
- `_edges`: список ребер в порядке добавления;
- `_n`: число вершин;
- `_m`: число ребер.

`addEdge(u, v)`:

1. Проверяет, что `0 <= u,v < _n`.
2. Добавляет `v` в `_matr[u]`.
3. Добавляет `u` в `_matr[v]`.
4. Добавляет пару `(u, v)` в `_edges`.
5. Увеличивает `_m`.

Граф не удаляет дубликаты ребер и не запрещает петли на уровне `Graph::addEdge`. Если входной JSON содержит такие ребра, они попадут в структуру.

Нюанс: `Graph()` объявлен как default constructor, но `_n` и `_m` в нем явно не инициализируются. В основном pipeline используется `Graph(n)`, поэтому это не проявляется.

### 7.4. `Embedding`

`Embedding` - координаты вершин графа:

```cpp
const Graph& _graph;
int32_t curDim;
std::vector<Pt> _coords;
```

Конструкторы:

| Конструктор | Поведение |
|---|---|
| `Embedding(graph)` | Создает `_coords` размера `graph.vertexCount()`, `curDim = 0`. |
| `Embedding(graph, dim)` | Создает `_coords` размера `graph.vertexCount()`, `curDim = dim`. |
| `Embedding(graph, startCoords)` | Копирует координаты, `curDim = startCoords[0].size()`. |

`setPos(v, pos)` проверяет:

- `pos.size() == curDim`;
- `v` находится в диапазоне вершин.

`Embedding` хранит ссылку на `Graph`, поэтому граф должен жить дольше вложения.

### 7.5. Начальная расстановка

Интерфейс:

```cpp
class InitialPlacementStrategy {
  virtual void computeInitial(Embedding& emb, const Space& space, const BorderPolicy& borderPolicy) const = 0;
};
```

Реализации:

| Стратегия | Поведение |
|---|---|
| `zero` | Всем вершинам ставит координату `(0, ..., 0)`. |
| `random` | Для каждой вершины вызывает `borderPolicy.randomPoint(space)`. |

`ZeroInitialPlacement` дополнительно выставляет `emb.curDim = space.dimension()`.

### 7.6. Border policy

Интерфейс `BorderPolicy`:

| Метод | Назначение |
|---|---|
| `force(space, point)` | Сила от границы. В текущем FaR не используется. |
| `normalizePoint(space, point)` | Вернуть точку в допустимую область. |
| `randomPoint(space)` | Случайная стартовая точка. |
| `domainVolume(space)` | Объем области для вычисления параметра `k` в FaR. |

Единственная доступная реализация - `DefaultBorderPolicy`.

`DefaultBorderPolicy`:

- хранит `std::mt19937 _rng`;
- `force(...)` возвращает нулевой вектор;
- `normalizePoint(...)` возвращает точку без изменений;
- `randomPoint(...)` генерирует каждую координату равномерно из `[-10, 10]`;
- `domainVolume(...)` возвращает `-1`.

Следствие для FaR: при default border policy объем области считается недоступным, поэтому `k` устанавливается в `1`, если не задана другая политика.

## 8. Пространства: интерфейс и общая математика

### 8.1. Интерфейс `Space`

`Space` задает геометрию, в которой живет вычислительное вложение:

| Метод | Смысл |
|---|---|
| `name()` | Имя пространства. |
| `dimension()` | Размерность координат `Pt`. |
| `dist(a,b)` | Расстояние между точками. |
| `norm(vec)` | Норма обычного вектора в координатах. |
| `logMap(from,to)` | Логарифмическое отображение: точка `to` как касательный вектор в `from`. |
| `expMap(from,tangent)` | Экспоненциальное отображение: шаг из `from` по касательному вектору. |
| `tangentNorm(at,tangent)` | Риманова норма касательного вектора в точке `at`. |
| `normalizePoint(p,figSize)` | Ограничение точки областью размера `figSize`. |
| `volume(figSize)` | Объем области размера `figSize`. |
| `areGeodesicSegmentsCrossing(a,b,c,d)` | Пересекаются ли геодезические отрезки `ab` и `cd`. |
| `isValid(c)` | Валидна ли координата как точка пространства. |

Идея для алгоритмов:

- алгоритм не должен знать формулы конкретного пространства;
- все геометрические формулы должны находиться в `Space` реализациях;
- layout работает через `dist`, `logMap`, `expMap`, `tangentNorm`.

### 8.2. `Space::determinantBareiss`

Метод вычисляет детерминант квадратной матрицы методом Барейса.

Для матрицы `A` размера `n x n`:

1. Проверяется квадратность.
2. На каждом шаге выбирается pivot с максимальным `|a[i][k]|` в текущем столбце.
3. При перестановке строк меняется знак детерминанта.
4. Выполняется обновление:

```text
a[i][j] = (a[i][j] * a[k][k] - a[i][k] * a[k][j]) / prev
```

где `prev` - предыдущий pivot, на первом шаге `prev = 1`.

Если pivot по модулю меньше `1e-18`, возвращается `0`.

Метод используется для вычисления объемов через матрицу Грама.

### 8.3. `Space::areEuclideanSegmentsCrossing2D`

Это общая проверка пересечения двух отрезков в 2D.

Для точек `a,b,c,d`:

- если отрезки имеют общий конец, возвращается `false`;
- если одна точка лежит на другом отрезке, возвращается `true`;
- иначе используются знаки ориентированных площадей.

Ориентированная площадь для векторов `u=(u_x,u_y)`, `v=(v_x,v_y)`:

```text
cross(u,v) = u_x v_y - u_y v_x
```

Скалярное произведение:

```text
dot(u,v) = u_x v_x + u_y v_y
```

Точка `p` лежит на отрезке `ab`, если:

```text
cross(b-a, p-a) = 0
dot(p-a, b-a) >= 0
dot(a-b, p-b) >= 0
```

## 9. EuclideanSpace

Файлы:

- `src/spaces/EuclideanSpace.h`
- `src/spaces/EuclideanSpace.cpp`

### 9.1. Модель координат

Точка пространства - вектор:

```text
x = (x_1, ..., x_d) in R^d
```

`isValid(x)` проверяет только размерность:

```text
size(x) == d
```

Проверки на `finite` в `EuclideanSpace::isValid` сейчас нет.

### 9.2. Расстояние и норма

Расстояние:

```text
dist(a,b) = sqrt(sum_i (a_i - b_i)^2)
```

Норма:

```text
norm(v) = sqrt(sum_i v_i^2)
```

### 9.3. Логарифмическое и экспоненциальное отображения

В евклидовом пространстве касательное пространство в любой точке отождествлено с `R^d`.

Логарифмическое отображение:

```text
log_from(to) = to - from
```

Экспоненциальное отображение:

```text
exp_from(v) = from + v
```

Риманова норма касательного вектора:

```text
tangentNorm(at, v) = norm(v)
```

Точка `at` проверяется на размерность, но в формуле не участвует.

### 9.4. Нормализация по `figSize`

`normalizePoint(p, figSize)` покоординатно обрезает точку в параллелепипед:

```text
res_i = min(figSize_i / 2, max(-figSize_i / 2, p_i))
```

То есть область:

```text
[-figSize_1/2, figSize_1/2] x ... x [-figSize_d/2, figSize_d/2]
```

### 9.5. Объем

Создается диагональная матрица Грама:

```text
G_ij = figSize_i^2, если i = j
G_ij = 0, если i != j
```

Возвращается:

```text
volume = sqrt(det(G))
```

Для положительных `figSize_i` это равно:

```text
volume = product_i figSize_i
```

### 9.6. Пересечения геодезических

Поддерживаются только `d = 2`.

Геодезики в Euclidean space - прямые отрезки, поэтому используется `Space::areEuclideanSegmentsCrossing2D`.

## 10. HyperbolicSpace

Файлы:

- `src/spaces/HyperbolicSpace.h`
- `src/spaces/HyperbolicSpace.cpp`

### 10.1. Модель координат

Класс называется `HyperbolicSpace`, а комментарий в заголовке говорит `Lorentzian space`.

В реализации точка хранится не как полный лоренцев вектор, а как пространственная часть гиперболоида:

```text
x = (x_1, ..., x_d) in R^d
```

Полная точка гиперболоида строится методом `lift`:

```text
X = lift(x) = (x_0, x_1, ..., x_d)
x_0 = sqrt(1 + sum_i x_i^2)
```

Тогда:

```text
-x_0^2 + x_1^2 + ... + x_d^2 = -1
x_0 > 0
```

Это верхний лист гиперболоида в пространстве Минковского.

`isValid(x)` проверяет:

- `size(x) == d`;
- все координаты конечны.

Любой конечный `x in R^d` валиден, потому что `lift(x)` всегда лежит на гиперболоиде.

### 10.2. Лоренцево произведение

Для полных векторов:

```text
A = (A_0, A_1, ..., A_d)
B = (B_0, B_1, ..., B_d)
```

реализовано:

```text
<A,B>_L = -A_0 B_0 + sum_{i=1}^{d} A_i B_i
```

Метод `lorentzProd` требует размерность `d + 1`.

### 10.3. Метод `lift`

Для координаты `x in R^d`:

```text
sqNorm = sum_{i=1}^{d} x_i^2
lift(x) = (sqrt(1 + sqNorm), x_1, ..., x_d)
```

### 10.4. Представление касательных векторов

Касательный вектор в коде тоже хранится как `Pt` длины `d`, то есть только пространственные координаты:

```text
v = (v_1, ..., v_d)
```

Чтобы получить полный лоренцев касательный вектор в точке `x`, используется `tangentToDefault(at, tangent)`:

```text
X = lift(x)
X_0 = sqrt(1 + ||x||^2)
V_0 = (x · v) / X_0
V_i = v_i
```

Полный вектор:

```text
V = (V_0, v_1, ..., v_d)
```

Он действительно лежит в касательном пространстве к гиперболоиду, потому что:

```text
<X,V>_L = -X_0 V_0 + x · v
         = -X_0 * (x · v / X_0) + x · v
         = 0
```

Обратная операция `defaultToTangent(def)` просто отбрасывает нулевую координату:

```text
defaultToTangent(W) = (W_1, ..., W_d)
```

### 10.5. Расстояние

Для `a,b in R^d`:

```text
A = lift(a)
B = lift(b)
alpha = -<A,B>_L
```

В координатах это:

```text
alpha = sqrt(1 + ||a||^2) sqrt(1 + ||b||^2) - a · b
```

Реализация обрезает:

```text
alpha = max(alpha, 1)
```

и возвращает:

```text
dist(a,b) = arcosh(alpha)
```

Это стандартное расстояние на гиперболоиде кривизны `-1`.

### 10.6. `norm`

`HyperbolicSpace::norm(vec)` - это евклидова норма координатного вектора:

```text
norm(v) = sqrt(sum_i v_i^2)
```

Это не общая риманова норма касательного вектора в произвольной точке. Для римановой нормы используется `tangentNorm(at, tangent)`.

### 10.7. Риманова норма касательного вектора

Для точки `x` и координатного касательного вектора `v`:

```text
V = tangentToDefault(x, v)
tangentNorm(x, v) = sqrt(max(<V,V>_L, 0))
```

В развернутом виде:

```text
<V,V>_L = -V_0^2 + sum_i v_i^2
V_0 = (x · v) / sqrt(1 + ||x||^2)
```

Значит:

```text
tangentNorm(x,v)^2 = ||v||^2 - (x · v)^2 / (1 + ||x||^2)
```

Код вычисляет это через `lorentzProd(V,V)`.

### 10.8. Логарифмическое отображение

`logMap(from, to)`:

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

Математически это:

```text
log_X(Y) = d * (Y - cosh(d) X) / sinh(d)
```

потому что `alpha = cosh(d)`, а `||Y - alpha X||_L = sinh(d)`.

В результате возвращаются только пространственные координаты касательного вектора.

### 10.9. Экспоненциальное отображение

`expMap(from, tangent)`:

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

Это стандартная формула экспоненты на гиперболоиде.

### 10.10. Нормализация точки

`normalizePoint(p, figSize)` ограничивает точку гиперболическим шаром вокруг начала.

Радиус:

```text
radius = min(figSize) / 2
```

Начало:

```text
center = (0, ..., 0)
```

Если:

```text
dist(center, p) <= radius
```

точка возвращается без изменений.

Иначе:

```text
v = logMap(center, p)
vN = tangentNorm(center, v)
return expMap(center, v / vN * radius)
```

То есть точка переносится на границу геодезического шара радиуса `radius` в направлении от центра к `p`.

Важно: текущий `DefaultBorderPolicy::normalizePoint` не вызывает `space.normalizePoint`; он возвращает точку как есть. Поэтому эта нормализация не используется в обычном запуске с `--borderPolicy default`.

### 10.11. Объем

`volume(figSize)` строит касательные базисные векторы в начале:

```text
e_i = (0, ..., figSize_i, ..., 0)
```

Далее строится матрица Грама через поляризационную формулу:

```text
G_ij = (||e_i + e_j||^2 - ||e_i||^2 - ||e_j||^2) / 2
```

где норма - `tangentNorm(origin, ...)`.

Возвращается:

```text
sqrt(det(G))
```

Так как точка - origin, эта величина фактически совпадает с объемом евклидова параллелепипеда в касательном пространстве. Это не интегральный объем гиперболического шара и не объем области, заданной границей в многообразии.

В текущем `DefaultBorderPolicy` этот метод не используется для FaR, потому что `domainVolume` возвращает `-1`.

### 10.12. Пересечения геодезических

Поддерживается только `d = 2`.

Для точки `p=(p_1,p_2)` строится модель Клейна:

```text
x_0 = sqrt(1 + p_1^2 + p_2^2)
K(p) = p / x_0
```

В модели Клейна гиперболические геодезики являются евклидовыми хордами. Поэтому:

```text
areGeodesicSegmentsCrossing(a,b,c,d)
  = areEuclideanSegmentsCrossing2D(K(a), K(b), K(c), K(d))
```

## 11. PoincareSpace

Файлы:

- `src/spaces/PoincareSpace.h`
- `src/spaces/PoincareSpace.cpp`

### 11.1. Назначение

`PoincareSpace` - пространство рисования после `PoincareProjection`. Оно используется не как вычислительное пространство `createSpace`, а как `drawing_space` в результате проекции.

Конструктор разрешает только:

```text
dim = 2
radius > 0
radius finite
```

### 11.2. Модель координат

Точка хранится в диске радиуса `R = _radius`:

```text
p = (p_x, p_y)
||p|| < R
```

Для формул точка переводится в единичный диск:

```text
x = p / R
```

Обратное преобразование:

```text
p = R x
```

`isValid(p)` проверяет:

- размерность `2`;
- конечность координат;
- `||p|| < R`.

### 11.3. Евклидова норма

Вспомогательная функция:

```text
euclideanNorm(p) = sqrt(p_x^2 + p_y^2)
```

Метод `norm(vec)` делает то же самое, но проверяет размерность и конечность.

Как и в `HyperbolicSpace`, `norm` не является римановой нормой в точке. Для этого есть `tangentNorm`.

### 11.4. Сложение Мебиуса

`mobiusAdd(a,b)` работает в единичном диске.

Обозначения:

```text
aSq = ||a||^2
bSq = ||b||^2
dot = a · b
den = 1 + 2 dot + aSq bSq
```

Если `den <= 0`, выбрасывается ошибка.

Результат:

```text
aCoeff = 1 + 2 dot + bSq
bCoeff = 1 - aSq

a ⊕ b = (aCoeff * a + bCoeff * b) / den
```

В координатной форме:

```text
(a ⊕ b)_i = ((1 + 2<a,b> + ||b||^2) a_i + (1 - ||a||^2) b_i)
            / (1 + 2<a,b> + ||a||^2 ||b||^2)
```

### 11.5. Расстояние

Для точек `A,B` в диске радиуса `R`:

```text
x = A / R
y = B / R
delta = (-x) ⊕ y
deltaNorm = min(||delta||, 1 - 16 epsilon)
dist(A,B) = 2 atanh(deltaNorm)
```

Радиус `R` используется как масштаб координат изображения. После деления на `R` геометрическая формула работает в единичном диске.

### 11.6. Логарифмическое отображение

Для `from = A`, `to = B`:

```text
x = A / R
y = B / R
delta = (-x) ⊕ y
deltaNorm = ||delta||
```

Если `deltaNorm` меньше `16 * epsilon`, возвращается нулевой вектор.

Далее:

```text
lambda = 2 / (1 - ||x||^2)
safeNorm = min(deltaNorm, 1 - EPS)
coeff = (2 / lambda) * atanh(safeNorm) / deltaNorm
```

Возвращается:

```text
R * coeff * delta
```

То есть касательный вектор возвращается в координатном масштабе диска радиуса `R`.

### 11.7. Экспоненциальное отображение

Для `from = A` и касательного вектора `tangent`:

```text
x = A / R
v = tangent / R
vNorm = ||v||
```

Если `vNorm < 16 * epsilon`, возвращается `from`.

Далее:

```text
lambda = 2 / (1 - ||x||^2)
step = (v / vNorm) * tanh(lambda * vNorm / 2)
result = R * (x ⊕ step)
```

После этого `result` нормализуется внутрь диска:

```text
normalizePoint(result, [2R, 2R])
```

### 11.8. Риманова норма касательного вектора

```text
x = at / R
lambda = 2 / (1 - ||x||^2)
tangentNorm(at, tangent) = lambda * ||tangent|| / R
```

Фактор `1/R` переводит координатный вектор из масштаба диска радиуса `R` в единичный диск.

### 11.9. Нормализация точки

`normalizePoint(p, figSize)`:

1. Проверяет размерность `p` и `figSize`.
2. Проверяет конечность координат.
3. Вычисляет `pointNorm = ||p||`.
4. Если:

```text
pointNorm < R * (1 - 1e-15)
```

возвращает `p`.

Иначе, если `pointNorm != 0`:

```text
return p / pointNorm * R * (1 - 1e-15)
```

Нюанс: значения `figSize` используются только для проверки длины, но не для вычисления радиуса. Реальный радиус берется из `_radius`.

### 11.10. Объем

`volume(figSize)` проверяет длину `figSize` и возвращает:

```text
infinity
```

В коде есть TODO: отделить объем многообразия от конечной области рисования.

### 11.11. Пересечения геодезических

Сначала точка переводится в единичный диск:

```text
u = p / R
```

Затем используется переход из модели Пуанкаре в модель Клейна:

```text
K(u) = 2u / (1 + ||u||^2)
```

После этого геодезические проверяются как евклидовы отрезки:

```text
areEuclideanSegmentsCrossing2D(K(a), K(b), K(c), K(d))
```

## 12. Поддержка пространств на практике

### 12.1. Пространства для вычислений

`createSpace` поддерживает:

| CLI `--space` | Реализовано в `createSpace` | Класс |
|---|---:|---|
| `euclidean` | Да | `EuclideanSpace` |
| `hyperbolic` | Да | `HyperbolicSpace` |
| `spherical` | Нет | - |

### 12.2. Пространства после проекции

| Проекция | Входное пространство | Выходное пространство |
|---|---|---|
| `identity` | `euclidean` | `EuclideanSpace(finalDim)` |
| `orthogonal` | `euclidean` | `EuclideanSpace(finalDim)` |
| `poincare` | `hyperbolic`, только `H2` | `PoincareSpace(2, radius)` |

### 12.3. Что сейчас не поддерживается

Сейчас не поддержаны:

- `hyperbolic dim=3 -> --2d`;
- `hyperbolic dim=4 -> --2d`;
- `hyperbolic dim=3 -> --3d` через Poincare ball;
- `spherical`;
- PoincareSpace размерности выше 2;
- общая проекция неевклидовых пространств в 2D/3D.

Для `hyperbolic dim > 2` теоретически можно добавить Poincare ball projection:

```text
x_0 = sqrt(1 + sum_i x_i^2)
y_i = R x_i / (x_0 + 1)
```

но после этого для 2D нужно отдельно выбрать способ снижения размерности: первые две координаты, PCA, камера, геодезическая плоскость и т.д. В текущем коде такой выбор не реализован.

## 13. Алгоритмы раскладки

### 13.1. Интерфейс `LayoutAlgorithm`

```cpp
virtual void computeLayout(
    Embedding& emb,
    const Space& space,
    const BorderPolicy& borderPolicy
) const = 0;
```

Алгоритм получает:

- изменяемое вложение;
- пространство, в котором лежат координаты;
- политику границ.

Фабрика `createLayoutAlgorithm` поддерживает:

| Имя | Класс |
|---|---|
| `random` | `RandomLayout` |
| `far` | `FruchtermanAndReingoldLayout` |

### 13.2. RandomLayout

`RandomLayout::computeLayout`:

1. Проверяет `emb.dimension() == space.dimension()`.
2. Для каждой вершины `v` устанавливает:

```text
pos(v) = borderPolicy.randomPoint(space)
```

При `DefaultBorderPolicy` каждая координата независимо равномерна на `[-10, 10]`.

### 13.3. FruchtermanAndReingoldLayout: параметры

Класс `FruchtermanAndReingoldLayout` реализует вариант Fruchterman-Reingold через операции пространства.

Параметры:

| Поле | Смысл | Default |
|---|---|---|
| `ITERS` | Число итераций. | `100` |
| `C` | Множитель идеальной длины. | `1.0` |

`computeLayout` просто вызывает `computeLayoutTest`.

### 13.4. Идеальная длина `k`

В начале:

```text
n = emb.size()
dim = space.dimension()
vol = borderPolicy.domainVolume(space)
```

Если `vol` не отрицателен с учетом `Comparator(EPS=1e-12)`, то:

```text
k = C * (vol / n)^(1 / dim)
```

Если `vol < 0`, то:

```text
k = 1
```

Для текущего `DefaultBorderPolicy`:

```text
domainVolume(space) = -1
```

поэтому в обычном запуске:

```text
k = 1
```

### 13.5. Силы притяжения и отталкивания

Используются классические функции Fruchterman-Reingold:

```text
f_a(x) = x^2 / k
f_r(x) = k^2 / x
```

где `x` - расстояние между вершинами в геометрии пространства.

### 13.6. Температура и охлаждение

Начальная температура:

```text
T = k
```

После каждой итерации:

```text
if it * 4 <= ITERS:
    T = T * 0.85
else:
    T = T * 0.95
```

В коде есть TODO: заменить это на более нормальную температуру.

### 13.7. Отталкивание

Для каждой пары вершин `i < j`:

```text
v1 = logMap(pos_i, pos_j)
v2 = logMap(pos_j, pos_i)
v1N = tangentNorm(pos_i, v1)
v2N = tangentNorm(pos_j, v2)
```

`v1` - касательный вектор в `pos_i`, направленный к `pos_j`.
`v2` - касательный вектор в `pos_j`, направленный к `pos_i`.

Если нормы не нулевые:

```text
disp[i] -= (v1 / v1N) * f_r(v1N)
disp[j] -= (v2 / v2N) * f_r(v2N)
```

Знак минус означает, что вершины отталкиваются друг от друга.

### 13.8. Притяжение по ребрам

Для каждого ребра `(i,j)`:

```text
v1 = logMap(pos_i, pos_j)
v2 = logMap(pos_j, pos_i)
v1N = tangentNorm(pos_i, v1)
v2N = tangentNorm(pos_j, v2)
```

Если нормы не нулевые:

```text
disp[i] += (v1 / v1N) * f_a(v1N)
disp[j] += (v2 / v2N) * f_a(v2N)
```

Знак плюс означает, что вершины движутся друг к другу вдоль геодезики.

### 13.9. Обновление координат

Для каждой вершины `i`:

```text
dispN = tangentNorm(pos_i, disp[i])
```

Если `dispN == 0`, вершина не двигается.

Иначе шаг ограничивается температурой:

```text
step = (disp[i] / dispN) * min(dispN, T)
```

Новая позиция:

```text
newPos = expMap(pos_i, step)
newPos = borderPolicy.normalizePoint(space, newPos)
pos_i = newPos
```

При `DefaultBorderPolicy` нормализация ничего не меняет.

### 13.10. Геометрическая обобщенность FaR

Алгоритм не использует напрямую евклидовы разности точек. Вместо этого он использует:

- `logMap` для направления вдоль геодезики;
- `tangentNorm` для длины касательного вектора;
- `expMap` для шага по многообразию.

Поэтому один и тот же алгоритм может работать в Euclidean и Hyperbolic space, пока соответствующий `Space` корректно реализует эти методы.

### 13.11. Legacy метод `computeLayoutEuclideanOld`

В классе есть старый метод `computeLayoutEuclideanOld`, но текущий pipeline его не вызывает.

Этот метод:

- использует обычные разности координат;
- руками ограничивает точку `figSize`;
- содержит `area = 0`, из-за чего `k = C * sqrt(area / n)` становится `0`;
- поэтому не должен рассматриваться как активная реализация алгоритма.

## 14. Проекции

### 14.1. Интерфейс `Projection`

```cpp
virtual ProjectionResult project(
    const Embedding& emb,
    const Space& space,
    const std::vector<int32_t>& figSize,
    int32_t finalDim
) const = 0;
```

`ProjectionResult` содержит:

```cpp
Embedding embedding;
SpacePtr space;
```

То есть проекция возвращает не только новые координаты, но и новое пространство, в котором эти координаты следует интерпретировать.

### 14.2. `fitToFigSize`

Общая вспомогательная функция для евклидовых проекций.

Проверки:

- `finalDim > 0`;
- `figSize.size() == finalDim`;
- все стороны `figSize` положительные;
- каждая координата конечна;
- размерность каждой точки равна `finalDim`.

Для каждой координаты ищутся:

```text
mns_i = min_p p_i
mxs_i = max_p p_i
len_i = mxs_i - mns_i
```

Выбирается единый масштаб:

```text
scale = min_i figSize_i / len_i
```

по всем осям, где `len_i > 0`.

Если все `len_i == 0`, то:

```text
scale = 1
```

Каждая точка центрируется по bounding box и масштабируется:

```text
res_i = (p_i - (mns_i + mxs_i) / 2) * scale
```

Итоговая область укладывается в:

```text
[-figSize_i/2, figSize_i/2]
```

### 14.3. IdentityProjection

Работает только для `space.name() == "euclidean"`.

Требует:

```text
finalDim == emb.dimension()
```

Действие:

1. Берет исходные координаты.
2. Вызывает `fitToFigSize`.
3. Возвращает `EuclideanSpace(finalDim)`.

### 14.4. OrthogonalProjection

Работает только для `space.name() == "euclidean"`.

Требует:

```text
finalDim <= emb.dimension()
```

Действие:

```text
res = (coord_1, ..., coord_finalDim)
```

То есть берутся первые `finalDim` координат, затем результат масштабируется через `fitToFigSize`.

### 14.5. PoincareProjection

Работает только для:

```text
space.name() == "hyperbolic"
emb.dimension() == 2
finalDim == 2
```

Если `emb.dimension() != 2`, будет ошибка:

```text
PoincareProjection::project: only H2 to 2D projection is supported
```

Радиус картинки:

```text
R = min(figSize[0], figSize[1]) / 2
```

Для точки гиперболоида, хранимой как пространственная часть:

```text
x = (x_1, x_2)
x_0 = sqrt(1 + x_1^2 + x_2^2)
```

Poincare coordinate:

```text
y_1 = R * x_1 / (x_0 + 1)
y_2 = R * x_2 / (x_0 + 1)
```

Это стандартная проекция из модели гиперболоида в диск Пуанкаре:

```text
y = R * x_spatial / (x_0 + 1)
```

Возвращаемое пространство:

```text
PoincareSpace(2, R)
```

Нюанс: если `figSize = [500, 300]`, радиус будет `150`, потому что берется минимум сторон. Диск помещается в меньшую сторону.

## 15. Метрики

Файлы:

- `src/metrics/Metrics.h`
- `src/metrics/Metrics.cpp`

Метрики считаются после проекции:

```cpp
Metrics metrics = computeMetrics(res.embedding, *res.space);
```

То есть для гиперболического запуска с `poincare` метрики считаются по координатам в `PoincareSpace`, а не по исходному `HyperbolicSpace`.

### 15.1. Поля `Metrics`

| Поле | Смысл в текущей реализации |
|---|---|
| `volume` | Координатный объем bounding box по точкам. |
| `minVertexDist` | Минимальное евклидово расстояние между координатами вершин. |
| `maxVertexDist` | Максимальное евклидово расстояние между координатами вершин. |
| `avgVertexDist` | Среднее евклидово расстояние между парами вершин. |
| `edgeCrossings` | Число пересечений геодезических ребер, только для `dim == 2`. |
| `minAngle` | Минимальный угол между смежными ребрами, только для `dim == 2`. |
| `maxAngle` | Максимальный угол между смежными ребрами, только для `dim == 2`. |
| `density` | `n / volume`, если `volume > 0`, иначе `0`. |
| `imageScore` | В C++ writer всегда записывается как `0`. |

### 15.2. Volume

Если `n > 0`, берется bounding box по координатам:

```text
mns_j = min_i p_i[j]
mxs_j = max_i p_i[j]
```

Объем:

```text
volume = product_j (mxs_j - mns_j)
```

Это не `space.volume`. Комментарий в коде прямо отмечает, что метрика пока закостылена на простую евклидову площадь/объем.

### 15.3. Vertex distances

Несмотря на наличие `space.dist`, расстояния между вершинами считаются евклидово по координатам:

```text
d(a,b) = sqrt(sum_i (a_i - b_i)^2)
```

Далее:

```text
minVertexDist = min_{i<j} d(p_i,p_j)
maxVertexDist = max_{i<j} d(p_i,p_j)
avgVertexDist = (sum_{i<j} d(p_i,p_j)) / (n(n-1)/2)
```

Если `n < 2`, все три величины равны `0`.

### 15.4. Density

```text
density = n / volume, если volume > 0
density = 0, иначе
```

Сравнение с нулем делается через `Comparator`.

### 15.5. Edge crossings

Считается только при:

```text
dim == 2
```

Для каждой пары ребер вызывается:

```text
space.areGeodesicSegmentsCrossing(a,b,c,d)
```

Реальное поведение зависит от `space`:

- в `EuclideanSpace` проверяются прямые отрезки;
- в `PoincareSpace` точки переводятся в Klein disk, где геодезики прямые;
- в `HyperbolicSpace` точки тоже переводятся в Klein coordinates.

Общие концы ребер не считаются пересечением, потому что базовая проверка возвращает `false` при совпадающих endpoints.

### 15.6. Angles

Углы считаются только при `dim == 2` и только для пар ребер, имеющих общую вершину.

Для центра `center` и двух соседних точек `a`, `b`:

```text
u = space.logMap(center, a)
v = space.logMap(center, b)
uNorm = space.tangentNorm(center, u)
vNorm = space.tangentNorm(center, v)
```

Если одна из норм нулевая, угол считается равным `0`.

Иначе скалярное произведение восстанавливается через норму суммы:

```text
sumNorm = space.tangentNorm(center, u + v)
scal = (sumNorm^2 - uNorm^2 - vNorm^2) / 2
```

Затем:

```text
cosValue = clamp(scal / (uNorm * vNorm), -1, 1)
angle = arccos(cosValue)
```

Это работает в любом пространстве, где `tangentNorm` задает норму от внутреннего произведения.

Нюанс: если `dim == 2`, но нет пар смежных ребер, `minAngle` может остаться равным начальному значению `1e18`.

## 16. I/O модули

### 16.1. JsonGraphReader

`JsonGraphReader` читает граф из JSON-файла.

Внутренние шаги:

1. `readFileToString(path)` читает файл целиком.
2. Если есть UTF-8 BOM, он удаляется.
3. `nlohmann::json::parse` парсит JSON.
4. Проверяется, что root - массив.
5. Ищется объект, где `"name" == graphName`.
6. Проверяется наличие массива `"edges"`.
7. Вычисляется число вершин.
8. Создается `Graph(n)`.
9. Каждое ребро добавляется через `Graph::addEdge`.

Проверки ребер:

- ребро должно быть массивом длины `2`;
- оба конца должны быть JSON integer;
- id вершины должен быть `0 <= id <= int32_t::max()`;
- после вычисления `n` endpoint должен быть `< n`.

### 16.2. EmbeddingWriter

`writeEmbeddingJson`:

1. Создает директорию результата, если она нужна.
2. Открывает `cfg.outputPath`.
3. Записывает config-поля.
4. Записывает вершины.
5. Записывает ребра.
6. Записывает метрики.

Координаты пишутся с `std::fixed << std::setprecision(6)`.

Для вершин:

- всегда пишутся `id`, `x`, `y`;
- `z` пишется только если `res.dimension() == 3`.

Следствие: writer предполагает, что итоговая размерность как минимум `2`. Это согласуется с текущим CLI, где есть только `--2d` и `--3d`.

## 17. Python-инструменты

### 17.1. `gen.py`

Назначение: генерировать графы в `samples/dataset.json` и сохранять PNG-эскизы.

Функции:

| Функция | Назначение |
|---|---|
| `load_json()` | Загружает существующий `samples/dataset.json` в глобальный `data`. |
| `upload_json()` | Перезаписывает `samples/dataset.json`. |
| `gen1(name,n,p)` | Генерирует граф Эрдеша-Реньи `nx.gnp_random_graph(n,p)`. |
| `gen2(name,szs,P)` | Генерирует stochastic block model. |

`gen1` и `gen2`:

1. Создают `networkx` граф.
2. Строят `spring_layout`.
3. Сохраняют PNG в `samples/<name>.png`.
4. Добавляют объект с `name`, `description`, `gen_algo`, `params`, `nodes`, `edges` в `data`.

### 17.2. `render.py`

Назначение: 2D-визуализация JSON, созданного C++ writer.

Основные шаги:

1. Читает result JSON.
2. Собирает `networkx.Graph`.
3. Читает позиции `x`, `y`.
4. Берет ребра из result JSON или, если их нет, восстанавливает из датасета.
5. Читает `fig_size` длины `2`.
6. Рисует граф.
7. Сохраняет PNG.

Для обычного пространства ребра рисуются как прямые:

```python
nx.draw_networkx_edges(...)
```

Для `drawing_space == "poincare"` ребра рисуются как геодезические диска Пуанкаре.

### 17.3. Геодезики в `render.py` для Poincare disk

Функция `poincare_geodesic(a,b,radius,samples=80)` строит дугу окружности, ортогональной границе диска.

Если точки лежат почти на одном диаметре:

```text
det = a_x b_y - a_y b_x
|det| <= 1e-12 * max(R^2, 1)
```

возвращается прямой отрезок.

Иначе центр окружности `C=(C_x,C_y)` находится из условий:

```text
||C - a||^2 = rho^2
||C - b||^2 = rho^2
||C||^2 = rho^2 + R^2
```

Из этого следуют линейные уравнения:

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

Далее выбирается короткая дуга между углами, но если середина дуги выходит за диск, sweep меняется на противоположный, чтобы рисовать дугу внутри диска.

### 17.4. Использование `fig_size` в `render.py`

`fig_size` используется для координатных пределов:

```text
xlim = [-fig_size[0]/2, fig_size[0]/2]
ylim = [-fig_size[1]/2, fig_size[1]/2]
```

Для Poincare:

```text
radius = min(fig_size) / 2
```

Размер matplotlib-фигуры при этом фиксирован:

```python
plt.subplots(figsize=(8, 8))
fig.savefig(..., dpi=180)
```

То есть `fig_size` влияет на масштаб координат, а не напрямую на количество пикселей PNG.

### 17.5. `render3d.py`

Назначение: 3D-визуализация result JSON с координатами `x`, `y`, `z`.

Особенности:

- требует у каждой вершины поля `id`, `x`, `y`, `z`;
- ребра рисуются 3D-линиями;
- вершины рисуются через `ax.scatter`;
- подписи рисуются через `ax.text`;
- если есть `fig_size` длины `3`, задаются `xlim`, `ylim`, `zlim`;
- `ax.set_box_aspect(fig_size)` сохраняет пропорции области.

### 17.6. `gd_experiments.py`

Это набор утилит для ноутбуков и серий экспериментов.

Основные возможности:

| Функция | Назначение |
|---|---|
| `setupPlotStyle()` | Настройка matplotlib-стиля. |
| `compileGd(repo)` | Запуск CMake configure/build. |
| `runGd(argv, repo, ...)` | Запуск `build/graph_drawing` с stdin для border policy и algorithm. |
| `runRender(repo,jsonPath,...)` | Запуск `render.py`. |
| `setImageScore(res, score)` | Ручная запись `imageScore` в JSON. |
| `compareResults(...)` | Табличное сравнение двух запусков. |
| `plotMetricComparison(...)` | Bar plot сравнения метрик. |
| `bestOfN(...)` | Серия запусков с scoring по весам метрик. |

`bestOfN`:

1. Делает `n` запусков.
2. Может менять seed как `baseSeed + i`.
3. Сохраняет результаты с суффиксом `_batch_i`.
4. Собирает метрики.
5. Нормирует вклад каждой выбранной метрики.
6. Возвращает лучший запуск, список scores и все runs.

## 18. Ноутбуки

В репозитории есть:

- `test.ipynb`;
- `far_test.ipynb`.

Это экспериментальные артефакты. Основная переиспользуемая логика для ноутбуков вынесена в `gd_experiments.py`.

## 19. Типовые команды

### 19.1. Справка

```bash
./build/graph_drawing --help
```

### 19.2. Euclidean random 2D

```bash
printf '\n' | ./build/graph_drawing \
  --graph SmallGraph \
  --algo random \
  --space euclidean \
  --initial-placement random \
  --dim 2 \
  --2d \
  --FS 100,100 \
  --seed 1
```

Здесь нужна одна пустая строка для `BorderPolicy default`.

### 19.3. Hyperbolic FaR через Poincare disk

```bash
printf '\n\n' | ./build/graph_drawing \
  --graph LargeGraph \
  --algo far \
  --space hyperbolic \
  --initial-placement random \
  --projection poincare \
  --dim 2 \
  --2d \
  --FS 500,500 \
  --seed 1543
```

Здесь:

- первая пустая строка - параметры `BorderPolicy default`;
- вторая пустая строка - параметры `FaR` по умолчанию.

### 19.4. Рендер результата

```bash
python3 render.py out/LargeGraph_far.json
```

или:

```bash
python3 render.py out/LargeGraph_far.json -o out/LargeGraph_far.png
```

## 20. Текущие ограничения и технические долги

### 20.1. Ограничения пространств и проекций

1. `spherical` указан в config, но не реализован.
2. `PoincareProjection` поддерживает только `H2 -> 2D`.
3. `PoincareSpace` поддерживает только `dim = 2`.
4. `IdentityProjection` и `OrthogonalProjection` поддерживают только Euclidean space.
5. Не реализована общая визуализация `hyperbolic dim > 2` в 2D/3D.

### 20.2. Ограничения metrics

1. `volume` считается как координатный bounding box, а не через `space.volume`.
2. Vertex distances считаются евклидово, а не через `space.dist`.
3. `edgeCrossings`, `minAngle`, `maxAngle` считаются только для `dim == 2`.
4. `imageScore` в C++ всегда `0`.
5. Если нет смежных пар ребер в 2D, `minAngle` может остаться `1e18`.

### 20.3. Ограничения border policy

1. Доступна только `default`.
2. `default` не ограничивает область.
3. `default` возвращает `domainVolume = -1`.
4. `force` определен в интерфейсе, но текущий FaR его не использует.

### 20.4. Ограничения CLI

1. `--FS 50 50` не работает, потому что `--FS` читает один argv-токен.
2. Параметры `far` и `borderPolicy` читаются из stdin, а не из обычного CLI.
3. `--space spherical` проходит список допустимых имен, но падает при создании пространства.
4. Ветка `poly` для border policy есть в парсере stdin, но недоступна как реальная policy.

### 20.5. Ограничения data/core

1. `Graph` не удаляет дубликаты ребер.
2. `Graph` не запрещает self-loop.
3. `Graph()` default constructor не инициализирует `_n` и `_m`.
4. `Embedding(graph,startCoords)` использует `startCoords[0]`, поэтому пустой `startCoords` небезопасен.

## 21. Рекомендации для развития проекта

### 21.1. Если добавлять новое пространство

Нужно реализовать новый класс в `src/spaces/`:

1. `name()`;
2. `dimension()`;
3. `dist`;
4. `norm`;
5. `logMap`;
6. `expMap`;
7. `tangentNorm`;
8. `normalizePoint`;
9. `volume`;
10. `areGeodesicSegmentsCrossing`;
11. `isValid`.

Затем добавить его в:

- `createSpace`;
- список допустимых `SPACE_NAMES`;
- документацию CLI.

Ключевой принцип: формулы геометрии должны оставаться внутри `Space`, а не переезжать в layout или metrics.

### 21.2. Если добавлять `hyperbolic dim > 2 -> 2D`

Возможный минимальный путь:

1. Обобщить гиперболоид -> Poincare ball:

```text
x_0 = sqrt(1 + sum_i x_i^2)
y_i = R x_i / (x_0 + 1)
```

2. Добавить явную стратегию снижения размерности:

```text
first coordinates
PCA
camera/projection plane
```

3. Явно назвать проекцию, чтобы пользователь понимал, что происходит.

Например:

- `poincare-first`;
- `poincare-pca`;
- `poincare-camera`.

### 21.3. Если исправлять metrics

Наиболее последовательное направление:

1. Для `minVertexDist`, `maxVertexDist`, `avgVertexDist` использовать `space.dist`.
2. Для `volume` решить, нужна ли:
   - координатная площадь картинки;
   - объем домена;
   - риманов объем пространства.
3. Для углов оставить текущую схему через `logMap` и `tangentNorm`, потому что она хорошо согласуется с интерфейсом `Space`.
4. Для 3D edge crossings определить математический смысл: пересечения отрезков в 3D обычно нестабильны и требуют отдельной постановки.

### 21.4. Если улучшать CLI

Варианты:

1. Перенести параметры `far` из stdin в CLI:

```bash
--far-C 1.0 --far-I 100
```

2. Разрешить `--FS` принимать несколько argv-токенов:

```bash
--FS 50 50
```

3. Убрать `spherical` из допустимых имен до реализации.
4. Убрать или реализовать `poly`.
5. Сделать ошибки автовыбора проекции более ранними и понятными.

## 22. Проверочная таблица методов пространств

### 22.1. EuclideanSpace

| Метод | Формула/поведение |
|---|---|
| `isValid(x)` | `size(x) == d`. |
| `dist(a,b)` | `sqrt(sum_i (a_i-b_i)^2)`. |
| `norm(v)` | `sqrt(sum_i v_i^2)`. |
| `logMap(from,to)` | `to - from`. |
| `expMap(from,v)` | `from + v`. |
| `tangentNorm(at,v)` | `norm(v)`. |
| `normalizePoint(p,FS)` | Clamp по каждой оси в `[-FS_i/2, FS_i/2]`. |
| `volume(FS)` | `sqrt(det(diag(FS_i^2)))`. |
| `areGeodesicSegmentsCrossing` | Только 2D, прямые отрезки. |

### 22.2. HyperbolicSpace

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

### 22.3. PoincareSpace

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

## 23. Итоговая картина проекта

Проект уже имеет хорошее разделение ответственности:

1. `Space` владеет геометрией.
2. `LayoutAlgorithm` работает через абстрактные геометрические операции.
3. `Projection` отделяет вычислительное пространство от пространства рисования.
4. `Metrics` агрегирует характеристики уже готового изображения.
5. Python-часть отделена от C++ ядра и отвечает за генерацию, рендер и эксперименты.

Самая сильная часть архитектуры - возможность писать layout один раз, а математические различия между Euclidean и Hyperbolic space прятать внутри `Space`.

Главные ограничения текущей версии:

1. Неевклидовые проекции реализованы только для `H2 -> Poincare 2D`.
2. Метрики частично евклидовы даже для Poincare/hyperbolic результатов.
3. Border policy пока фактически не ограничивает область.
4. CLI содержит несколько заготовок, которые еще не реализованы полностью.

Для отчета или защиты проекта особенно важно подчеркивать, что гиперболический layout действительно работает в модели гиперболоида через `logMap`, `expMap` и `tangentNorm`, а диск Пуанкаре сейчас используется именно как пространство рисования после проекции.
