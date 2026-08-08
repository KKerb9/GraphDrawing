from __future__ import annotations

import json
import math
import subprocess
import sys
from pathlib import Path
from typing import Any, Callable, Dict, Iterable, Iterator, Optional, Tuple, Union

import matplotlib.pyplot as plt
import numpy as np
import pandas as pd
import seaborn as sns


metricKeys = (
	"volume",
	"minVertexDist",
	"maxVertexDist",
	"avgVertexDist",
	"minEdgeVertexDist",
	"edgeCrossings",
	"minAngle",
	"maxAngle",
	"density",
	"imageScore",
)

algoStdinDefaults = {
	"far": "\n",
}

borderStdinDefaults = {
	"default": "\n",
	"poly": "\n",
}


FlagValues = Union[Dict[str, Any], list[Any]]


def setupPlotStyle() -> None:
	"""Применяет общий Seaborn-стиль для графиков экспериментов.

	Функция меняет глобальные настройки Matplotlib/Seaborn и должна вызываться
	перед построением графика. При импорте модуля стиль не применяется.
	"""
	sns.set_theme(
		style="darkgrid",
		palette="deep",
		rc={
			"figure.facecolor": "#1A1A1A",
			"axes.facecolor": "#222121",
			"axes.edgecolor": "#eeeeee",
			"axes.labelcolor": "#eeeeee",
			"xtick.color": "#eeeeee",
			"ytick.color": "#eeeeee",
			"text.color": "#eeeeee",
			"grid.color": "#555555",
			"font.family": ["Arial Rounded MT Bold", "Avenir", "DejaVu Sans"],
		},
	)


def binaryPath(repo: Path) -> Path:
	"""Возвращает путь к бинарю graph_drawing в каталоге сборки `repo`.

	Аргумент `repo` — корень репозитория с каталогом `build`.
	"""
	return repo / "build" / "graph_drawing"


def compileGd(repo: Path, buildDir: str = "build") -> None:
	"""Конфигурирует и собирает CMake-проект.

	Аргумент `repo` задаёт корень репозитория, а `buildDir` — имя или путь
	к каталогу CMake относительно него.
	"""
	buildPath = repo / buildDir
	buildPath.mkdir(parents=True, exist_ok=True)
	subprocess.run(["cmake", "-S", str(repo), "-B", str(buildPath)], cwd=repo, check=True)
	subprocess.run(["cmake", "--build", str(buildPath)], cwd=repo, check=True)


def flagsToArgv(flags: FlagValues) -> list[str]:
	"""Возвращает CLI-аргументы запуска в исходном порядке.

	Основной формат `flags` — список токенов, например
	`["--graph", "graph1", "--2d"]`. Dict поддерживается для совместимости:
	его ключи добавляются в порядке вставки, а значение `None` передаёт флаг
	без отдельного аргумента.
	"""
	if isinstance(flags, dict):
		argv: list[str] = []
		for flag, value in flags.items():
			argv.append(str(flag))
			if value is not None:
				argv.append(str(value))
		return argv
	if isinstance(flags, list):
		return [str(value) for value in flags]
	raise TypeError("flagsToArgv: flags must be dict or list")


def copyFlags(flags: FlagValues) -> FlagValues:
	"""Создаёт независимую копию CLI-флагов, сохраняя их исходный формат.

	Для dict возвращается dict, для списка — новый список токенов. Это нужно,
	чтобы callback мог переиспользовать и менять свой список между запусками.
	"""
	if isinstance(flags, dict):
		return dict(flags)
	if isinstance(flags, list):
		return list(flags)
	raise TypeError("copyFlags: flags must be dict or list")


def isCliFlag(value: Any) -> bool:
	"""Проверяет, является ли токен началом длинного CLI-флага.

	Значения вроде `-1` не считаются флагами: они могут быть аргументами
	`--seed`, `--FS` и других параметров.
	"""
	return isinstance(value, str) and value.startswith("--")


def flagValue(flags: FlagValues, flag: str, default: Any = None) -> Any:
	"""Возвращает последнее значение CLI-флага или `default`.

	В списке токенов значением считается следующий токен, если он не начинается
	с `--`; последнее повторное указание флага работает так же, как в CLI.
	"""
	if isinstance(flags, dict):
		return flags.get(flag, default)
	if isinstance(flags, list):
		value = default
		for index, current in enumerate(flags):
			if current == flag and index + 1 < len(flags) and not isCliFlag(flags[index + 1]):
				value = flags[index + 1]
		return value
	raise TypeError("flagValue: flags must be dict or list")


def setFlagValue(flags: FlagValues, flag: str, value: Any) -> FlagValues:
	"""Записывает значение CLI-флага в независимый набор флагов.

	Для списка заменяется аргумент последнего `flag`, либо пара добавляется в
	конец. Это сохраняет порядок, которым пользуется notebook.
	"""
	if isinstance(flags, dict):
		flags[flag] = value
		return flags
	if isinstance(flags, list):
		for index in range(len(flags) - 1, -1, -1):
			if flags[index] != flag:
				continue
			if index + 1 < len(flags) and not isCliFlag(flags[index + 1]):
				flags[index + 1] = str(value)
			else:
				flags.insert(index + 1, str(value))
			return flags
		flags.extend([flag, str(value)])
		return flags
	raise TypeError("setFlagValue: flags must be dict or list")


def outputJsonPath(repo: Path, flags: FlagValues) -> Path:
	"""Возвращает абсолютный путь к JSON, который должен создать запуск.

	Если `--output` отсутствует, путь строится как
	`out/<graph>_<algo>.json` относительно `repo`.
	"""
	output = flagValue(flags, "--output")
	if output is not None:
		path = Path(output)
		return path if path.is_absolute() else repo / path
	graphName = flagValue(flags, "--graph", "SmallGraph")
	algoName = flagValue(flags, "--algo", "random")
	return repo / "out" / f"{graphName}_{algoName}.json"


def stdinLine(flags: FlagValues, inputFlags: Optional[FlagValues], kind: str) -> Optional[str]:
	"""Строит строку stdin для алгоритма или border policy.

	`inputFlags` — список токенов интерактивного ввода, например
	`["--C", "1.0", "--I", "100"]`. Пустой список, пустой dict и `None`
	используют текущие настройки по умолчанию. `kind` должен быть `"algo"`
	или `"border"`.
	"""
	if kind == "algo":
		default = algoStdinDefaults.get(str(flagValue(flags, "--algo", "random")))
	elif kind == "border":
		default = borderStdinDefaults.get(str(flagValue(flags, "--borderPolicy", "default")))
	else:
		raise ValueError("stdinLine: kind must be 'algo' or 'border'")
	if not inputFlags:
		return default
	return " ".join(flagsToArgv(inputFlags)) + "\n"


def runTest(
	repo: Path,
	flags: FlagValues,
	*,
	algoFlags: Optional[FlagValues] = None,
	borderFlags: Optional[FlagValues] = None,
) -> pd.DataFrame:
	"""Выполняет один запуск graph_drawing и возвращает его результат в DataFrame.

	`flags` — упорядоченный список CLI-токенов запуска. `algoFlags` и
	`borderFlags` — такие же списки для второй и первой строки stdin.
	Dict остаётся доступным только для совместимости со старым кодом.
	Результат всегда содержит одну строку с метриками, путями, stdout/stderr
	и копиями применённых списков. Координаты и рёбра остаются в JSON по `jsonPath`.
	"""
	currentFlags = copyFlags(flags)
	currentAlgoFlags = None if algoFlags is None else copyFlags(algoFlags)
	currentBorderFlags = None if borderFlags is None else copyFlags(borderFlags)
	borderLine = stdinLine(currentFlags, currentBorderFlags, "border")
	algoLine = stdinLine(currentFlags, currentAlgoFlags, "algo")
	stdin = None if borderLine is None and algoLine is None else (borderLine or "") + (algoLine or "")
	command = [str(binaryPath(repo))] + flagsToArgv(currentFlags)
	if stdin is None:
		process = subprocess.run(command, cwd=repo, capture_output=True, text=True, stdin=subprocess.DEVNULL)
	else:
		process = subprocess.run(command, cwd=repo, capture_output=True, text=True, input=stdin)

	jsonPath = outputJsonPath(repo, currentFlags)
	data: Dict[str, Any] = {}
	if jsonPath.is_file():
		with open(jsonPath, encoding="utf-8") as f:
			data = json.load(f)
	metrics = data.get("metrics", {})
	row = {key: metrics.get(key, np.nan) for key in metricKeys}
	row.update({
		"run": 1,
		"returnCode": process.returncode,
		"stdout": process.stdout or "",
		"stderr": process.stderr or "",
		"jsonPath": str(jsonPath),
		"pngPath": None,
		"seed": data.get("seed", flagValue(currentFlags, "--seed")),
		"stdin": stdin,
		"flags": currentFlags,
		"algoFlags": currentAlgoFlags,
		"borderFlags": currentBorderFlags,
	})
	return pd.DataFrame([row])


def flagsForBatchRun(repo: Path, flags: FlagValues, runIndex: int, outDir: Optional[Union[str, Path]], baseSeed: Optional[int]) -> FlagValues:
	"""Создаёт независимый список CLI-флагов для одного запуска серии.

	`runIndex` начинается с нуля и используется в имени `_batch_i`. `outDir`
	переопределяет каталог JSON, а `baseSeed` при наличии заменяет `--seed`
	значением `baseSeed + runIndex`.
	"""
	currentFlags = copyFlags(flags)
	if baseSeed is not None:
		setFlagValue(currentFlags, "--seed", baseSeed + runIndex)
	output = flagValue(currentFlags, "--output")
	if output is None:
		graphName = flagValue(currentFlags, "--graph", "SmallGraph")
		algoName = flagValue(currentFlags, "--algo", "random")
		name = f"{graphName}_{algoName}.json"
		batchDir = Path(outDir) if outDir is not None else repo / "out" / "batch_runs"
	else:
		outputPath = Path(output)
		name = outputPath.name
		batchDir = Path(outDir) if outDir is not None else outputPath.parent
		if not batchDir.is_absolute():
			batchDir = repo / batchDir
	namePath = Path(name)
	setFlagValue(currentFlags, "--output", str(batchDir / f"{namePath.stem}_batch_{runIndex}{namePath.suffix}"))
	return currentFlags


def nextFlags(iterator: Optional[Iterator[FlagValues]], default: Optional[FlagValues]) -> Optional[FlagValues]:
	"""Берёт следующие CLI-флаги из iterator или копию базового списка.

	После окончания iterator используется `default`. Функция копирует список
	сразу после получения, поэтому следующая итерация generator не меняет уже
	подготовленный запуск.
	"""
	if iterator is not None:
		try:
			return copyFlags(next(iterator))
		except StopIteration:
			pass
	return None if default is None else copyFlags(default)


def runTests(
	repo: Path,
	flags: FlagValues,
	n: int,
	*,
	baseSeed: Optional[int] = None,
	outDir: Optional[Union[str, Path]] = None,
	flagsForRun: Optional[Callable[[range], Iterable[FlagValues]]] = None,
	algoFlags: Optional[FlagValues] = None,
	algoFlagsForRun: Optional[Callable[[range], Iterable[FlagValues]]] = None,
	borderFlags: Optional[FlagValues] = None,
	borderFlagsForRun: Optional[Callable[[range], Iterable[FlagValues]]] = None,
) -> pd.DataFrame:
	"""Выполняет `n` запусков и собирает все исходные результаты в DataFrame.

	`flags` задаёт базовый список CLI-токенов. Callback-функции `*ForRun` получают
	`range(n)` и выдают списки для отдельных запусков; после их окончания
	используется базовый список.
	`baseSeed` создаёт последовательность `baseSeed + i`, а `outDir` задаёт
	каталог для JSON серии. Метрики не усредняются и не оцениваются этой функцией.
	"""
	if n <= 0:
		raise ValueError("runTests: n must be positive")
	seed = baseSeed if baseSeed is not None else flagValue(flags, "--seed")
	if seed is not None:
		seed = int(seed)
		if seed < 0 or seed + n - 1 > 2**32 - 1:
			raise ValueError("runTests: seed sequence must fit uint32")
	runIndices = range(n)
	flagsIterator = iter(flagsForRun(runIndices)) if flagsForRun is not None else None
	algoIterator = iter(algoFlagsForRun(runIndices)) if algoFlagsForRun is not None else None
	borderIterator = iter(borderFlagsForRun(runIndices)) if borderFlagsForRun is not None else None
	runs: list[pd.DataFrame] = []
	for i in range(n):
		currentFlags = nextFlags(flagsIterator, flags) or []
		currentFlags = flagsForBatchRun(repo, currentFlags, i, outDir, seed)
		currentAlgoFlags = nextFlags(algoIterator, algoFlags)
		currentBorderFlags = nextFlags(borderIterator, borderFlags)
		res = runTest(repo, currentFlags, algoFlags=currentAlgoFlags, borderFlags=currentBorderFlags)
		res.loc[:, "run"] = i + 1
		runs.append(res)
	return pd.concat(runs, ignore_index=True)


def scoreRuns(df: pd.DataFrame, weights: Dict[str, float]) -> pd.DataFrame:
	"""Возвращает копию результатов с итоговой колонкой `score`.

	`weights` — map весов метрик: положительный вес максимизирует метрику,
	отрицательный минимизирует её, нулевой игнорируется. Нормализация выполняется
	только внутри `df`; исходный DataFrame не изменяется.
	"""
	res = df.copy()
	scores = np.zeros(len(res), dtype=float)
	for key in metricKeys:
		weight = float(weights.get(key, 0.0))
		if weight == 0:
			continue
		values = res[key].to_numpy(dtype=float)
		valueMin = np.min(values)
		valueMax = np.max(values)
		if valueMin == valueMax:
			scores += abs(weight)
			continue
		normalized = (values - valueMin) / (valueMax - valueMin)
		scores += weight * normalized if weight > 0 else abs(weight) * (1.0 - normalized)
	res["score"] = scores
	return res


def bestRun(df: pd.DataFrame) -> pd.DataFrame:
	"""Возвращает однострочный DataFrame с максимальным `score`.

	Аргумент `df` должен быть результатом `scoreRuns`; без колонки `score`
	функция сообщает об ошибке.
	"""
	if "score" not in df:
		raise ValueError("bestRun: df must contain score")
	return df.loc[[df["score"].idxmax()]].copy()


def setImageScore(df: pd.DataFrame, run: int, score: float) -> pd.DataFrame:
	"""Устанавливает ручную метрику `imageScore` для запуска в DataFrame.

	`run` — значение колонки `run`, а `score` — конечная числовая оценка.
	JSON-файл запуска не изменяется. Функция меняет переданный DataFrame и
	возвращает его для удобного продолжения цепочки вызовов.
	"""
	if not math.isfinite(float(score)):
		raise ValueError("setImageScore: score must be finite")
	mask = df["run"] == run
	if not mask.any():
		raise ValueError("setImageScore: run not found")
	df.loc[mask, "imageScore"] = float(score)
	return df


def renderRun(
	repo: Path,
	df: pd.DataFrame,
	run: int,
	*,
	pngPath: Optional[Union[str, Path]] = None,
	dataset: Optional[str] = None,
) -> Path:
	"""Рендерит JSON указанного запуска и возвращает путь к PNG.

	`run` выбирает строку по колонке `run`. `pngPath` задаёт путь изображения;
	без него PNG создаётся рядом с JSON. `dataset` передаётся в `render.py`,
	когда для визуализации требуется исходный датасет. DataFrame и JSON не
	изменяются. Возвращаемый `Path` можно сразу передать в `displayTwo`.
	"""
	rows = df.index[df["run"] == run]
	if len(rows) != 1:
		raise ValueError("renderRun: run not found or not unique")
	rowIndex = rows[0]
	jsonPath = Path(df.at[rowIndex, "jsonPath"])
	if pngPath is None:
		resolvedPngPath = jsonPath.parent / f"{jsonPath.stem}.png"
	else:
		resolvedPngPath = Path(pngPath)
		if not resolvedPngPath.is_absolute():
			resolvedPngPath = repo / resolvedPngPath
	args = [sys.executable, str(repo / "render.py"), str(jsonPath), "-o", str(resolvedPngPath)]
	if dataset is not None:
		args += ["--dataset", str(dataset)]
	subprocess.run(args, cwd=repo, check=True)
	return resolvedPngPath


def displayImagePath(image: Any) -> Path:
	"""Возвращает путь к изображению для `displayTwo`.

	`image` может быть строкой, `Path` или объектом `IPython.display.Image` с
	атрибутом `filename`. Другие типы не содержат достаточно данных, чтобы
	прочитать изображение с диска.
	"""
	if isinstance(image, (str, Path)):
		return Path(image)
	filename = getattr(image, "filename", None)
	if filename:
		return Path(filename)
	raise TypeError("displayTwo: image must be a path or IPython.display.Image with a filename")


def displayTwo(
	image1: Any,
	image2: Any,
	*,
	title1: Optional[str] = None,
	title2: Optional[str] = None,
	title: Optional[str] = None,
	figSize: Tuple[float, float] = (14, 7),
) -> Tuple[plt.Figure, Any]:
	"""Показывает два PNG-изображения графов рядом в одной Figure.

	`image1` и `image2` принимают путь к PNG либо объект
	`IPython.display.Image`, созданный из файла. `title1` и `title2` задают
	подписи отдельных изображений, `title` — общий заголовок, а `figSize` —
	размер Figure в дюймах. Без отдельных подписей используются имена файлов.
	Возвращает созданные Figure и две оси Matplotlib.
	"""
	setupPlotStyle()
	paths = [displayImagePath(image1), displayImagePath(image2)]
	for path in paths:
		if not path.is_file():
			raise FileNotFoundError(f"displayTwo: image not found: {path}")
	fig, axes = plt.subplots(1, 2, figsize=figSize)
	for ax, path, imageTitle in zip(axes, paths, (title1, title2)):
		ax.imshow(plt.imread(path))
		ax.set_title(imageTitle or path.stem, fontsize=15, fontweight="bold", pad=12)
		ax.axis("off")
	if title is not None:
		fig.suptitle(title, fontsize=22, fontweight="bold", y=.98)
		fig.tight_layout(rect=(0, 0, 1, .94))
	else:
		fig.tight_layout()
	plt.show()
	return fig, axes


def infoText(info: Optional[Union[str, Dict[str, Any]]]) -> Optional[str]:
	"""Преобразует строку или map подписи серии в текст для блока графика.

	Строка отображается без изменений. Для dict каждая пара превращается в
	строку `ключ: значение` в порядке вставки. `None` отключает блок подписи.
	"""
	if info is None:
		return None
	if isinstance(info, str):
		return info
	if isinstance(info, dict):
		return "\n".join(f"{key}: {value}" for key, value in info.items())
	raise TypeError("infoText: info must be str, dict or None")


def failedRunsMessage(df: pd.DataFrame) -> Optional[str]:
	"""Возвращает краткое пояснение, если все строки DataFrame завершились ошибкой.

	Используется перед визуализацией, чтобы вместо пустых осей показать stderr
	запусков. При отсутствии колонки `returnCode` или хотя бы одном успешном
	запуске функция возвращает `None`.
	"""
	if "returnCode" not in df or df.empty:
		return None
	returnCodes = pd.to_numeric(df["returnCode"], errors="coerce")
	if (returnCodes == 0).any():
		return None
	message = "plotMetrics: все запуски завершились с ошибкой"
	if "stderr" not in df:
		return message
	errors = []
	for stderr in df["stderr"].dropna().astype(str):
		lines = [line for line in stderr.splitlines() if line.strip()]
		if lines and lines[-1] not in errors:
			errors.append(lines[-1])
	if errors:
		return message + ": " + " | ".join(errors[:3])
	return message


def plotMetrics(
	df: pd.DataFrame,
	metric: str = "all",
	*,
	columns: int = 3,
	title: Optional[str] = None,
	info: Optional[Union[str, Dict[str, Any]]] = None,
) -> Tuple[plt.Figure, Any]:
	"""Строит значения одной метрики или всех метрик по номерам запусков.

	`metric` принимает ключ из `metricKeys` либо `"all"`. `columns` задаёт
	число колонок сетки для всех метрик. По оси X используется `run`, а по Y —
	исходные значения без усреднения. `title` задаёт заголовок Figure, а `info`
	добавляет отдельный блок: строку либо map, например `{"Граф": graphName}`.
	Если все запуски завершились ошибкой или у запрошенной метрики нет чисел,
	функция сообщает об этом вместо пустого графика. Возвращаются Figure и axes.
	"""
	setupPlotStyle()
	if df.empty:
		raise ValueError("plotMetrics: df is empty")
	if "run" not in df:
		raise ValueError("plotMetrics: df must contain run")
	failedMessage = failedRunsMessage(df)
	if failedMessage is not None:
		raise ValueError(failedMessage)
	if metric == "all":
		metrics = list(metricKeys)
	elif metric in metricKeys:
		metrics = [metric]
	else:
		raise ValueError("plotMetrics: unknown metric")
	if columns <= 0:
		raise ValueError("plotMetrics: columns must be positive")
	plotDf = df.copy()
	availableMetrics = []
	for key in metrics:
		if key not in plotDf:
			plotDf[key] = np.nan
		plotDf[key] = pd.to_numeric(plotDf[key], errors="coerce")
		if plotDf[key].notna().any():
			availableMetrics.append(key)
	if not availableMetrics:
		raise ValueError("plotMetrics: requested metrics contain no numeric values")
	infoBlock = infoText(info)
	boxStyle = {
		"facecolor": "#222121",
		"edgecolor": "#eeeeee",
		"alpha": 0.9,
		"boxstyle": "round,pad=0.45",
	}
	if len(metrics) == 1:
		fig, ax = plt.subplots(figsize=(9, 5))
		sns.lineplot(data=plotDf, x="run", y=metrics[0], marker="o", markersize=5, linewidth=1.6, estimator=None, errorbar=None, ax=ax)
		ax.set_xlabel("Номер запуска")
		ax.set_ylabel(metrics[0])
		ax.set_title(title or metrics[0], fontsize=18, fontweight="bold", pad=14)
		if infoBlock is not None:
			ax.text(0.02, 0.98, infoBlock, transform=ax.transAxes, va="top", ha="left", bbox=boxStyle)
		fig.tight_layout()
		plt.show()
		return fig, ax
	columns = min(columns, len(metrics))
	rows = math.ceil(len(metrics) / columns)
	fig, axes = plt.subplots(rows, columns, figsize=(5 * columns, 3.8 * rows), squeeze=False)
	for ax, key in zip(axes.flat, metrics):
		if key in availableMetrics:
			sns.lineplot(data=plotDf, x="run", y=key, marker="o", markersize=4, linewidth=1.4, estimator=None, errorbar=None, ax=ax)
		else:
			ax.text(0.5, 0.5, "Нет числовых данных", transform=ax.transAxes, ha="center", va="center")
		ax.set_xlabel("Номер запуска")
		ax.set_ylabel(key)
		ax.set_title(key)
	for ax in list(axes.flat)[len(metrics):]:
		ax.axis("off")
	top = 1.0
	if title:
		fig.suptitle(title, fontsize=22, fontweight="bold", y=0.995)
		top = 0.96
	if infoBlock is not None:
		fig.text(0.99, 0.982, infoBlock, va="top", ha="right", bbox=boxStyle)
		top = min(top, 0.94)
	if top < 1.0:
		fig.tight_layout(rect=(0, 0, 1, top))
	else:
		fig.tight_layout()
	plt.show()
	return fig, axes
