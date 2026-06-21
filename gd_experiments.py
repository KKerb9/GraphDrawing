from __future__ import annotations

import json
import math
import subprocess
import sys
from pathlib import Path
from typing import Any, Callable, Dict, Iterator, List, Optional, Sequence, Tuple, Union

import pandas as pd

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

stdinDefault = {
	"far": "\n",
}

currentResult: Optional[Dict[str, Any]] = None

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
	algoFlags: Optional[Sequence[Any]] = None,
) -> Dict[str, Any]:
	global currentResult

	command = [str(binaryPath(repo))] + list(argv)
	line = algoFlagsToStdinLine(argv, algoFlags)
	if isinstance(line, bytes):
		line = line.decode(errors="replace")
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
		"algoFlags": None if algoFlags is None else list(algoFlags),
		"algoFlagsRepr": algoFlagsRepr,
		"stdinRepr": line,
	}
	currentResult = result
	return result


def setImageScore(imageScore: float) -> None:
	global currentResult

	if currentResult is None:
		raise RuntimeError("setImageScore: runGd must be called first")
	if isinstance(imageScore, bool) or not isinstance(imageScore, (int, float)):
		raise TypeError("setImageScore: imageScore must be a number")
	imageScore = float(imageScore)
	if not math.isfinite(imageScore):
		raise ValueError("setImageScore: imageScore must be finite")

	jsonPath = Path(currentResult["jsonPath"])
	if not jsonPath.is_file():
		raise FileNotFoundError("setImageScore: result JSON not found: " + str(jsonPath))

	with open(jsonPath, encoding="utf-8") as f:
		data = json.load(f)
	metrics = data.setdefault("metrics", {})
	if not isinstance(metrics, dict):
		raise ValueError("setImageScore: JSON metrics must be an object")

	metrics["imageScore"] = imageScore
	currentResult["imageScore"] = imageScore
	currentResult["metrics"] = metrics
	currentResult["json"] = data

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


def experimentRow(
	metrics: Dict[str, Any],
	argvRepr: Optional[str] = None,
	algoFlagsRepr: Optional[str] = None,
	stdinRepr: Optional[str] = None,
	jsonPath: Optional[Union[str, Path]] = None,
	pngPath: Optional[Union[str, Path]] = None,
	note: Optional[str] = None,
	extra: Optional[Dict[str, Any]] = None,
) -> pd.Series:
	row = {k: metrics.get(k) for k in metricKeys}
	row["argvRepr"] = argvRepr
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
		".gd-log-pairs{border-collapse:collapse}"
		".gd-log-pairs th{border:1px solid #555;padding:6px 10px;text-align:left}"
		".gd-log-pairs td{border:1px solid #ccc;padding:6px 10px;text-align:left;vertical-align:top}"
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
) -> Tuple[Dict[str, Any], List[float], List[Dict[str, Any]]]:
	global currentResult

	"""
	algoFlags — на каждый прогон, если algoFlagsForRun не задан.
	algoFlagsForRun(iters) — функция из ноутбука: iters == n, внутри yield на прогон;
	вызывается как iter(algoFlagsForRun(n)). Если yield кончились раньше n — дальше algoFlags.
	baseSeed задаёт seeds baseSeed + i; без него используется --seed из argv или случайные seeds.
	weights — коэффициенты по ключам из metricKeys; 0 пропускается, знак задаёт направление оптимизации.
	"""
	if n <= 0:
		raise ValueError("bestOfN: n must be positive")
	graphName, algoName, _ = parseArgs(argv)
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
	outDir = Path(outDir or repo / "out" / "batch_runs")
	outDir.mkdir(parents=True, exist_ok=True)
	algoFlagsIterator = iter(algoFlagsForRun(n)) if algoFlagsForRun is not None else None
	runs = []
	for i in range(n):
		out = outDir / f"{graphName}_{algoName}_batch_{i}.json"
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
		currentAlgoFlags = algoFlags
		if algoFlagsIterator is not None:
			try:
				currentAlgoFlags = next(algoFlagsIterator)
			except StopIteration:
				currentAlgoFlags = algoFlags
		runs.append(runGd(currentArgv, repo, algoFlags=currentAlgoFlags))

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
