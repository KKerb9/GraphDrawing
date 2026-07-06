from __future__ import annotations

import json
import math
import subprocess
import sys
from pathlib import Path
from typing import Any, Callable, Dict, Iterator, List, Optional, Sequence, Tuple, Union

import pandas as pd
import matplotlib.pyplot as plt
from tqdm.auto import tqdm

metricKeys = (
	"volume",
	"minVertexDist",
	"maxVertexDist",
	"avgVertexDist",
	"edgeCrossings",
	"minAngle",
	"maxAngle",
	"density",
	"imageScore",
)

algosWithStdin = frozenset({"far"})
borderPoliciesWithStdin = frozenset({"default", "poly"})

stdinDefault = {
	"far": "\n",
}

borderStdinDefault = {
	"default": "\n",
	"poly": "\n",
}

currentResult: Optional[Dict[str, Any]] = None

plotColors = ("#4C78A8", "#F58518", "#54A24B", "#E45756", "#72B7B2")


def setupPlotStyle() -> None:
	plt.rcParams.update({
		"figure.figsize": (9, 6),
		"figure.dpi": 120,
		"savefig.dpi": 160,
		"font.size": 12,
		"axes.titlesize": 15,
		"axes.labelsize": 12,
		"axes.grid": True,
		"grid.alpha": 0.25,
		"grid.linestyle": "--",
		"axes.spines.top": False,
		"axes.spines.right": False,
		"legend.frameon": False,
		"axes.prop_cycle": plt.cycler(color=plotColors),
	})
	try:
		from IPython import get_ipython
		shell = get_ipython()
		if shell is not None:
			shell.run_line_magic("config", "InlineBackend.figure_format = 'retina'")
	except ImportError:
		pass

# argv: аргументы CLI (как sys.argv без имени программы).
# algoFlags: последовательность значений для stdin (или None — взять stdinDefault для алгоритма).
def algoFlagsToStdinLine(argv: Sequence[str], algoFlags: Optional[Sequence[Any]]) -> Optional[str]:
	_, algoName, _ = parseArgs(argv)
	if algoFlags is None:
		return stdinDefault.get(algoName)
	if not algoFlags:
		if algoName in algosWithStdin:
			return stdinDefault.get(algoName, "\n")
		return None
	return " ".join(str(x) for x in algoFlags) + "\n"


# borderFlags: первая строка stdin (или None — взять borderStdinDefault для border policy).
def borderFlagsToStdinLine(argv: Sequence[str], borderFlags: Optional[Sequence[Any]]) -> Optional[str]:
	borderPolicyName = "default"
	for i, arg in enumerate(argv):
		if arg == "--borderPolicy" and i + 1 < len(argv):
			borderPolicyName = argv[i + 1]
	if borderFlags is None:
		return borderStdinDefault.get(borderPolicyName)
	if not borderFlags:
		if borderPolicyName in borderPoliciesWithStdin:
			return borderStdinDefault.get(borderPolicyName, "\n")
		return None
	return " ".join(str(x) for x in borderFlags) + "\n"


def binaryPath(repo: Path) -> Path:
	return repo / "build" / "graph_drawing"


def compileGd(repo: Path, buildDir: str = "build") -> None:
	b = repo / buildDir
	b.mkdir(parents=True, exist_ok=True)
	subprocess.run(["cmake", "-S", str(repo), "-B", str(b)], cwd=repo)
	subprocess.run(["cmake", "--build", str(b)], cwd=repo)


def parseArgs(argv: Sequence[str]) -> Tuple[str, str, Optional[str]]:
	graphName, algoName, output = "SmallGraph", "random", None
	i = 0
	while i < len(argv):
		if argv[i] == "--graph" and i + 1 < len(argv):
			graphName = argv[i + 1]
			i += 2
		elif argv[i] == "--algo" and i + 1 < len(argv):
			algoName = argv[i + 1]
			i += 2
		elif argv[i] == "--output" and i + 1 < len(argv):
			output = argv[i + 1]
			i += 2
		else:
			i += 1
	return graphName, algoName, output


def outputJsonPath(repo: Path, argv: Sequence[str]) -> Path:
	graphName, algoName, output = parseArgs(argv)
	if output:
		p = Path(output)
		return p if p.is_absolute() else repo / p
	return repo / "out" / f"{graphName}_{algoName}.json"


def runGd(
	argv: Sequence[str],
	repo: Path,
	borderFlags: Optional[Sequence[Any]] = None,
	algoFlags: Optional[Sequence[Any]] = None,
) -> Dict[str, Any]:
	global currentResult

	command = [str(binaryPath(repo))] + list(argv)
	borderLine = borderFlagsToStdinLine(argv, borderFlags)
	algoLine = algoFlagsToStdinLine(argv, algoFlags)
	line = None if borderLine is None and algoLine is None else (borderLine or "") + (algoLine or "")
	if line is None:
		process = subprocess.run(command, cwd=repo, capture_output=True, text=True, stdin=subprocess.DEVNULL)
	else:
		process = subprocess.run(command, cwd=repo, capture_output=True, text=True, input=line)

	path = outputJsonPath(repo, argv)
	data = {}
	if path.is_file():
		with open(path, encoding="utf-8") as f:
			data = json.load(f)
	metrics = data.setdefault("metrics", {}) if data else {}

	borderFlagsRepr = None if borderFlags is None else " ".join(str(x) for x in borderFlags)
	algoFlagsRepr = None if algoFlags is None else " ".join(str(x) for x in algoFlags)

	result = {
		"returnCode": process.returncode,
		"stdout": process.stdout or "",
		"stderr": process.stderr or "",
		"jsonPath": path,
		"metrics": metrics,
		"json": data,
		"seed": data.get("seed") if data else None,
		"argv": list(argv),
		"argvRepr": " ".join(argv),
		"borderFlags": None if borderFlags is None else list(borderFlags),
		"borderFlagsRepr": borderFlagsRepr,
		"algoFlags": None if algoFlags is None else list(algoFlags),
		"algoFlagsRepr": algoFlagsRepr,
		"borderStdinRepr": borderLine,
		"algoStdinRepr": algoLine,
		"stdinRepr": line,
	}
	currentResult = result
	return result


def setImageScore(res: Dict[str, Any], imageScore: float) -> None:
	if not isinstance(res, dict):
		raise TypeError("setImageScore: res must be a dictionary returned by runGd")
	if isinstance(imageScore, bool) or not isinstance(imageScore, (int, float)):
		raise TypeError("setImageScore: imageScore must be a number")
	imageScore = float(imageScore)
	if not math.isfinite(imageScore):
		raise ValueError("setImageScore: imageScore must be finite")

	if res.get("jsonPath") is None:
		raise ValueError("setImageScore: res has no jsonPath")
	jsonPath = Path(res["jsonPath"])
	if not jsonPath.is_file():
		raise FileNotFoundError("setImageScore: result JSON not found: " + str(jsonPath))

	with open(jsonPath, encoding="utf-8") as f:
		data = json.load(f)
	metrics = data.setdefault("metrics", {})
	if not isinstance(metrics, dict):
		raise ValueError("setImageScore: JSON metrics must be an object")

	metrics["imageScore"] = imageScore
	res["imageScore"] = imageScore
	res["metrics"] = metrics
	res["json"] = data

	with open(jsonPath, "w", encoding="utf-8") as f:
		json.dump(data, f, ensure_ascii=False, indent=2)
		f.write("\n")


def runRender(
	repo: Path,
	jsonPath: Union[str, Path],
	pngPath: Optional[Union[str, Path]] = None,
	dataset: Optional[str] = None,
) -> Path:
	jp = Path(jsonPath)
	args = [sys.executable, str(repo / "render.py"), str(jp)]
	if dataset is not None:
		args += ["--dataset", str(dataset)]
	if pngPath is not None:
		args += ["-o", str(pngPath)]
	subprocess.run(args, cwd=repo)
	if pngPath:
		return Path(pngPath)
	return jp.parent / f"{jp.stem}.png"


def displayTwo(
	image1: Any,
	image2: Any,
	*,
	title1: Optional[str] = None,
	title2: Optional[str] = None,
	title: Optional[str] = None,
	figSize: Tuple[float, float] = (14, 7),
):
	def imagePath(image: Any) -> Path:
		if isinstance(image, (str, Path)):
			return Path(image)
		filename = getattr(image, "filename", None)
		if filename:
			return Path(filename)
		raise TypeError("displayTwo: image must be a path or IPython.display.Image with a filename")

	paths = [imagePath(image1), imagePath(image2)]
	for path in paths:
		if not path.is_file():
			raise FileNotFoundError("displayTwo: image not found: " + str(path))

	fig, axes = plt.subplots(1, 2, figsize=figSize)
	for ax, path, imageTitle in zip(axes, paths, [title1, title2]):
		ax.imshow(plt.imread(path))
		ax.set_title(imageTitle or path.stem, pad=10)
		ax.axis("off")
	if title:
		fig.suptitle(title, fontsize=17)
	fig.tight_layout(rect=(0, 0, 1, 0.96) if title else None)
	plt.show()
	return fig, axes


def experimentRow(
	metrics: Dict[str, Any],
	argvRepr: Optional[str] = None,
	borderFlagsRepr: Optional[str] = None,
	algoFlagsRepr: Optional[str] = None,
	stdinRepr: Optional[str] = None,
	jsonPath: Optional[Union[str, Path]] = None,
	pngPath: Optional[Union[str, Path]] = None,
	note: Optional[str] = None,
	extra: Optional[Dict[str, Any]] = None,
) -> pd.Series:
	row = {k: metrics.get(k) for k in metricKeys}
	row["argvRepr"] = argvRepr
	row["borderFlagsRepr"] = borderFlagsRepr
	row["algoFlagsRepr"] = algoFlagsRepr
	row["stdinRepr"] = stdinRepr
	row["jsonPath"] = str(jsonPath) if jsonPath else None
	row["pngPath"] = str(pngPath) if pngPath else None
	row["note"] = note
	if extra:
		row.update(extra)
	return pd.Series(row)


logRowExtraKeys = (
	"argvRepr",
	"borderFlagsRepr",
	"algoFlagsRepr",
	"stdinRepr",
	"jsonPath",
	"pngPath",
	"note",
)


def logRowAsPairs(
	row: Union[pd.Series, pd.DataFrame],
	*,
	rowIndex: int = -1,
	field: str = "поле",
	value: str = "значение",
) -> pd.DataFrame:
	"""Одна строка лога → таблица из двух столбцов (название поля, значение), по строке на поле."""
	if isinstance(row, pd.DataFrame):
		row = row.iloc[rowIndex]
	ordered: List[str] = []
	seen: set[str] = set()
	for k in list(metricKeys) + list(logRowExtraKeys):
		if k in row.index and k not in seen:
			ordered.append(k)
			seen.add(k)
	for k in row.index:
		if k not in seen:
			ordered.append(k)
			seen.add(k)
	return pd.DataFrame({field: ordered, value: [row[k] for k in ordered]})


def resultAsPairs(
	res: Dict[str, Any],
	*,
	pngPath: Optional[Union[str, Path]] = None,
	note: Optional[str] = None,
	field: str = "поле",
	value: str = "значение",
) -> pd.DataFrame:
	resolvedPngPath = pngPath if pngPath is not None else res.get("pngPath")
	if resolvedPngPath is None and res.get("jsonPath") is not None:
		jsonPath = Path(res["jsonPath"])
		resolvedPngPath = jsonPath.parent / f"{jsonPath.stem}.png"
	row = experimentRow(
		res.get("metrics", {}),
		argvRepr=res.get("argvRepr"),
		borderFlagsRepr=res.get("borderFlagsRepr"),
		algoFlagsRepr=res.get("algoFlagsRepr"),
		stdinRepr=res.get("stdinRepr"),
		jsonPath=res.get("jsonPath"),
		pngPath=resolvedPngPath,
		note=note if note is not None else res.get("note"),
	)
	return logRowAsPairs(row, field=field, value=value)


def compareResults(
	res1: Dict[str, Any],
	res2: Dict[str, Any],
	*,
	field: str = "поле",
	res1Label: str = "res1",
	res2Label: str = "res2",
) -> pd.DataFrame:
	def resultRow(res: Dict[str, Any]) -> pd.Series:
		return experimentRow(
			res.get("metrics", {}),
			argvRepr=res.get("argvRepr"),
			borderFlagsRepr=res.get("borderFlagsRepr"),
			algoFlagsRepr=res.get("algoFlagsRepr"),
			stdinRepr=res.get("stdinRepr"),
			jsonPath=res.get("jsonPath"),
			pngPath=res.get("pngPath"),
			note=res.get("note"),
		)

	row1 = resultRow(res1)
	row2 = resultRow(res2)
	ordered = list(metricKeys) + list(logRowExtraKeys)
	return pd.DataFrame({
		field: ordered,
		res1Label: [row1[k] for k in ordered],
		res2Label: [row2[k] for k in ordered],
	})


def plotMetricComparison(
	res1: Dict[str, Any],
	res2: Dict[str, Any],
	*,
	metrics: Optional[Sequence[str]] = None,
	res1Label: str = "res1",
	res2Label: str = "res2",
	title: str = "Сравнение метрик",
	columns: int = 3,
):
	metrics = list(metrics or metricKeys)
	values = []
	for metric in metrics:
		value1 = res1.get("metrics", {}).get(metric)
		value2 = res2.get("metrics", {}).get(metric)
		if isinstance(value1, (int, float)) and isinstance(value2, (int, float)):
			values.append((metric, float(value1), float(value2)))
	if not values:
		raise ValueError("plotMetricComparison: no numeric metrics")

	columns = min(columns, len(values))
	rows = math.ceil(len(values) / columns)
	fig, axes = plt.subplots(rows, columns, figsize=(4.2 * columns, 3.3 * rows), squeeze=False)
	for ax, (metric, value1, value2) in zip(axes.flat, values):
		bars = ax.bar([res1Label, res2Label], [value1, value2], color=plotColors[:2], width=0.62)
		ax.set_title(metric)
		ax.grid(axis="y")
		ax.bar_label(bars, fmt="%.4g", padding=3, fontsize=10)
		ax.margins(y=0.18)
	for ax in list(axes.flat)[len(values):]:
		ax.axis("off")
	fig.suptitle(title, fontsize=17)
	fig.tight_layout(rect=(0, 0, 1, 0.96))
	plt.show()
	return fig, axes


def plotRunScores(
	scores: Sequence[float],
	*,
	title: str = "Оценки запусков",
):
	if not scores:
		raise ValueError("plotRunScores: scores must not be empty")
	values = [float(score) for score in scores]
	runs = list(range(1, len(values) + 1))
	bestIndex = max(range(len(values)), key=values.__getitem__)

	fig, ax = plt.subplots(figsize=(9, 5))
	ax.plot(runs, values, marker="o", linewidth=2, markersize=6, label="score")
	ax.scatter(
		[runs[bestIndex]],
		[values[bestIndex]],
		color=plotColors[1],
		s=90,
		zorder=3,
		label="лучший",
	)
	ax.set_title(title)
	ax.set_xlabel("Запуск")
	ax.set_ylabel("Score")
	ax.set_xticks(runs)
	ax.grid(axis="y")
	ax.legend()
	fig.tight_layout()
	plt.show()
	return fig, ax


def logDfAsPairs(
	logDf: pd.DataFrame,
	*,
	field: str = "поле",
	value: str = "значение",
	runLabel: str = "запуск",
) -> pd.DataFrame:
	"""Несколько строк лога: колонка runLabel + поле + значение (длинный формат)."""
	parts: List[pd.DataFrame] = []
	for i in range(len(logDf)):
		t = logRowAsPairs(logDf.iloc[i], field=field, value=value)
		t.insert(0, runLabel, i)
		parts.append(t)
	return pd.concat(parts, ignore_index=True)


def styleLogPairsTable(df: pd.DataFrame):
	"""Границы и отступы для отображения в Jupyter.

	Без pandas Styler (ему нужен jinja2): HTML + to_html.
	"""
	from IPython.display import HTML

	table = df.to_html(index=False, classes="gd-log-pairs", border=0, escape=True)
	css = (
		"<style>"
		".gd-log-pairs{border:1px solid #9aa9bc;border-collapse:separate;border-spacing:0;width:100%;font-size:13px}"
		".gd-log-pairs th{background:#e9eff7;border-bottom:2px solid #9aa9bc;padding:8px 12px;text-align:left}"
		".gd-log-pairs td{border-bottom:1px solid #d9e0e8;padding:7px 12px;text-align:left;vertical-align:top}"
		".gd-log-pairs tbody tr:nth-child(even){background:#f7f9fc}"
		".gd-log-pairs tbody tr:hover{background:#edf3fa}"
		"</style>"
	)
	return HTML(css + table)


def appendLogRow(
	logDf: Optional[pd.DataFrame],
	metrics: Dict[str, Any],
	**kwargs: Any,
) -> pd.DataFrame:
	appendCsv = kwargs.pop("appendCsv", None)
	row = experimentRow(metrics, **kwargs)
	nextDf = pd.DataFrame([row]) if logDf is None or logDf.empty else pd.concat([logDf, row.to_frame().T], ignore_index=True)
	if appendCsv is not None:
		path = Path(appendCsv)
		path.parent.mkdir(parents=True, exist_ok=True)
		nextDf.tail(1).to_csv(path, mode="a", header=not path.is_file(), index=False)
	return nextDf


def metricContribs(vals: Sequence[float], w: float) -> List[float]:
	if w == 0:
		return [0.0] * len(vals)
	vmin, vmax = min(vals), max(vals)
	if vmin == vmax:
		return [abs(w)] * len(vals)
	raw = [(v - vmin) / (vmax - vmin) for v in vals]
	if w > 0:
		return [w * r for r in raw]
	aw = abs(w)
	return [aw * (1.0 - r) for r in raw]


def bestOfN(
	repo: Path,
	argv: Sequence[str],
	n: int,
	weights: Dict[str, float],
	algoFlags: Optional[Sequence[Any]] = None,
	outDir: Optional[Union[str, Path]] = None,
	algoFlagsForRun: Optional[Callable[[int], Iterator[Optional[Sequence[Any]]]]] = None,
	baseSeed: Optional[int] = None,
	borderFlags: Optional[Sequence[Any]] = None,
	borderFlagsForRun: Optional[Callable[[int], Iterator[Optional[Sequence[Any]]]]] = None,
) -> Tuple[Dict[str, Any], List[float], List[Dict[str, Any]]]:
	global currentResult

	"""
	algoFlags — на каждый прогон, если algoFlagsForRun не задан.
	algoFlagsForRun(iters) — функция из ноутбука: iters == n, внутри yield на прогон;
	вызывается как iter(algoFlagsForRun(n)). Если yield кончились раньше n — дальше algoFlags.
	borderFlags и borderFlagsForRun работают аналогично для первой строки stdin.
	baseSeed задаёт seeds baseSeed + i; без него используется --seed из argv или случайные seeds.
	Имена результатов строятся из --output с суффиксом _batch_i; outDir переопределяет только каталог.
	weights — коэффициенты по ключам из metricKeys; 0 пропускается, знак задаёт направление оптимизации.
	"""
	graphName, algoName, output = parseArgs(argv)
	argvBaseSeed = None
	if baseSeed is None:
		for seedIndex, arg in enumerate(argv):
			if arg != "--seed":
				continue
			if seedIndex + 1 >= len(argv):
				raise ValueError("bestOfN: --seed requires a value")
			try:
				argvBaseSeed = int(argv[seedIndex + 1])
			except (TypeError, ValueError) as e:
				raise ValueError("bestOfN: --seed must be a uint32 value") from e
	resolvedBaseSeed = baseSeed if baseSeed is not None else argvBaseSeed
	if resolvedBaseSeed is not None:
		if isinstance(resolvedBaseSeed, bool) or not isinstance(resolvedBaseSeed, int):
			raise TypeError("bestOfN: baseSeed must be an integer")
		if resolvedBaseSeed < 0 or resolvedBaseSeed + n - 1 > 2**32 - 1:
			raise ValueError("bestOfN: seed sequence must fit uint32")
	if output is not None:
		outputPath = Path(output)
		batchDir = Path(outDir) if outDir is not None else outputPath.parent
		batchName = outputPath.name
	else:
		batchDir = Path(outDir) if outDir is not None else repo / "out" / "batch_runs"
		batchName = f"{graphName}_{algoName}.json"
	batchDirOnDisk = batchDir if batchDir.is_absolute() else repo / batchDir
	batchDirOnDisk.mkdir(parents=True, exist_ok=True)
	batchNamePath = Path(batchName)
	borderFlagsIterator = iter(borderFlagsForRun(n)) if borderFlagsForRun is not None else None
	algoFlagsIterator = iter(algoFlagsForRun(n)) if algoFlagsForRun is not None else None
	runs = []
	for i in tqdm(range(n), desc="bestOfN " + batchNamePath.stem, unit="run"):
		out = batchDir / f"{batchNamePath.stem}_batch_{i}{batchNamePath.suffix}"
		currentArgv = list(argv)
		if resolvedBaseSeed is not None:
			withoutSeed = []
			argIndex = 0
			while argIndex < len(currentArgv):
				if currentArgv[argIndex] == "--seed":
					argIndex += 2
					continue
				withoutSeed.append(currentArgv[argIndex])
				argIndex += 1
			currentArgv = withoutSeed + ["--seed", str(resolvedBaseSeed + i)]
		if "--output" in currentArgv:
			j = currentArgv.index("--output")
			currentArgv[j + 1] = str(out)
		else:
			currentArgv += ["--output", str(out)]
		currentBorderFlags = borderFlags
		if borderFlagsIterator is not None:
			try:
				currentBorderFlags = next(borderFlagsIterator)
			except StopIteration:
				currentBorderFlags = borderFlags
		currentAlgoFlags = algoFlags
		if algoFlagsIterator is not None:
			try:
				currentAlgoFlags = next(algoFlagsIterator)
			except StopIteration:
				currentAlgoFlags = algoFlags
		runs.append(runGd(
			currentArgv,
			repo,
			borderFlags=currentBorderFlags,
			algoFlags=currentAlgoFlags,
		))

	keys = [k for k in metricKeys if weights.get(k)]
	if not keys:
		keys = list(metricKeys)
	scores = [0.0] * n
	for k in keys:
		w = weights.get(k, 0.0)
		if not w: continue
		vals = []
		for r in runs:
			v = r["metrics"].get(k)
			vals.append(float(v) if v is not None else 0.0)
		for i, c in enumerate(metricContribs(vals, w)):
			scores[i] += c
	bestI = max(range(n), key=lambda j: scores[j])
	currentResult = runs[bestI]
	return runs[bestI], scores, runs
