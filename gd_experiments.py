from __future__ import annotations

import json
import subprocess
import sys
from pathlib import Path
from typing import Any, Callable, Dict, Iterator, List, Optional, Sequence, Tuple, Union

import pandas as pd

METRIC_KEYS = (
	"volume",
	"minVertexDist",
	"maxVertexDist",
	"avgVertexDist",
	"edgeCrossings",
	"minAngle",
	"maxAngle",
	"density",
)

ALGOS_WITH_STDIN = frozenset({"far"})

STDIN_DEFAULT = {
	"far": "\n",
}

# argv: аргументы CLI (как sys.argv без имени программы).
# algo_flags: последовательность значений для stdin (или None — взять STDIN_DEFAULT для алгоритма).
def algoFlagsToStdinLine(argv: Sequence[str], algo_flags: Optional[Sequence[Any]]) -> Optional[str]:
	_, a, _ = _parse(argv)
	if algo_flags is None:
		return STDIN_DEFAULT.get(a)
	if not algo_flags:
		if a in ALGOS_WITH_STDIN:
			return STDIN_DEFAULT.get(a, "\n")
		return None
	return " ".join(str(x) for x in algo_flags) + "\n"


def binaryPath(repo: Path) -> Path:
	return repo / "build" / "graph_drawing"


def compileGd(repo: Path, build_dir: str = "build") -> None:
	b = repo / build_dir
	b.mkdir(parents=True, exist_ok=True)
	subprocess.run(["cmake", "-S", str(repo), "-B", str(b)], cwd=repo)
	subprocess.run(["cmake", "--build", str(b)], cwd=repo)


def _parse(argv: Sequence[str]) -> Tuple[str, str, Optional[str]]:
	g, a, out = "SmallGraph", "random", None
	i = 0
	while i < len(argv):
		if argv[i] == "--graph" and i + 1 < len(argv):
			g = argv[i + 1]
			i += 2
		elif argv[i] == "--algo" and i + 1 < len(argv):
			a = argv[i + 1]
			i += 2
		elif argv[i] == "--output" and i + 1 < len(argv):
			out = argv[i + 1]
			i += 2
		else:
			i += 1
	return g, a, out


def outputJsonPath(repo: Path, argv: Sequence[str]) -> Path:
	g, a, out = _parse(argv)
	if out:
		p = Path(out)
		return p if p.is_absolute() else repo / p
	return repo / "out" / f"{g}_{a}.json"


def runGd(
	argv: Sequence[str],
	repo: Path,
	algo_flags: Optional[Sequence[Any]] = None,
) -> Dict[str, Any]:
	cmd = [str(binaryPath(repo))] + list(argv)
	line = algoFlagsToStdinLine(argv, algo_flags)
	if isinstance(line, bytes):
		line = line.decode(errors="replace")
	if line is None:
		p = subprocess.run(cmd, cwd=repo, capture_output=True, text=True, stdin=subprocess.DEVNULL)
	else:
		p = subprocess.run(cmd, cwd=repo, capture_output=True, text=True, input=line)

	path = outputJsonPath(repo, argv)
	data = {}
	if path.is_file():
		with open(path, encoding="utf-8") as f:
			data = json.load(f)
	metrics = data.get("metrics") or {}

	af_repr = None if algo_flags is None else " ".join(str(x) for x in algo_flags)

	return {
		"returncode": p.returncode,
		"stdout": p.stdout or "",
		"stderr": p.stderr or "",
		"json_path": path,
		"metrics": metrics,
		"json": data,
		"argv": list(argv),
		"argv_repr": " ".join(argv),
		"algo_flags": None if algo_flags is None else list(algo_flags),
		"algo_flags_repr": af_repr,
		"stdin_repr": line,
	}


def runRender(
	repo: Path,
	json_path: Union[str, Path],
	png_path: Optional[Union[str, Path]] = None,
	dataset: Optional[str] = None,
) -> Path:
	jp = Path(json_path)
	args = [sys.executable, str(repo / "render.py"), str(jp)]
	if dataset is not None:
		args += ["--dataset", str(dataset)]
	if png_path is not None:
		args += ["-o", str(png_path)]
	subprocess.run(args, cwd=repo)
	if png_path:
		return Path(png_path)
	return jp.parent / f"{jp.stem}.png"


def experimentRow(
	metrics: Dict[str, Any],
	image_score: Optional[float] = None,
	argv_repr: Optional[str] = None,
	algo_flags_repr: Optional[str] = None,
	stdin_repr: Optional[str] = None,
	json_path: Optional[Union[str, Path]] = None,
	png_path: Optional[Union[str, Path]] = None,
	note: Optional[str] = None,
	extra: Optional[Dict[str, Any]] = None,
) -> pd.Series:
	row = {k: metrics.get(k) for k in METRIC_KEYS}
	row["image_score"] = image_score
	row["argv_repr"] = argv_repr
	row["algo_flags_repr"] = algo_flags_repr
	row["stdin_repr"] = stdin_repr
	row["json_path"] = str(json_path) if json_path else None
	row["png_path"] = str(png_path) if png_path else None
	row["note"] = note
	if extra:
		row.update(extra)
	return pd.Series(row)


LOG_ROW_EXTRA_KEYS = (
	"image_score",
	"argv_repr",
	"algo_flags_repr",
	"stdin_repr",
	"json_path",
	"png_path",
	"note",
)


def logRowAsPairs(
	row: Union[pd.Series, pd.DataFrame],
	*,
	row_index: int = -1,
	field: str = "поле",
	value: str = "значение",
) -> pd.DataFrame:
	"""Одна строка лога → таблица из двух столбцов (название поля, значение), по строке на поле."""
	if isinstance(row, pd.DataFrame):
		row = row.iloc[row_index]
	ordered: List[str] = []
	seen: set[str] = set()
	for k in list(METRIC_KEYS) + list(LOG_ROW_EXTRA_KEYS):
		if k in row.index and k not in seen:
			ordered.append(k)
			seen.add(k)
	for k in row.index:
		if k not in seen:
			ordered.append(k)
			seen.add(k)
	return pd.DataFrame({field: ordered, value: [row[k] for k in ordered]})


def logDfAsPairs(
	log_df: pd.DataFrame,
	*,
	field: str = "поле",
	value: str = "значение",
	run_label: str = "запуск",
) -> pd.DataFrame:
	"""Несколько строк лога: колонка run_label + поле + значение (длинный формат)."""
	parts: List[pd.DataFrame] = []
	for i in range(len(log_df)):
		t = logRowAsPairs(log_df.iloc[i], field=field, value=value)
		t.insert(0, run_label, i)
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
	log_df: Optional[pd.DataFrame],
	metrics: Dict[str, Any],
	**kwargs: Any,
) -> pd.DataFrame:
	append_csv = kwargs.pop("append_csv", None)
	row = experimentRow(metrics, **kwargs)
	next_df = pd.DataFrame([row]) if log_df is None or log_df.empty else pd.concat([log_df, row.to_frame().T], ignore_index=True)
	if append_csv is not None:
		path = Path(append_csv)
		path.parent.mkdir(parents=True, exist_ok=True)
		next_df.tail(1).to_csv(path, mode="a", header=not path.is_file(), index=False)
	return next_df


def _metricContribs(vals: Sequence[float], w: float) -> List[float]:
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
	algo_flags: Optional[Sequence[Any]] = None,
	out_dir: Optional[Union[str, Path]] = None,
	algoFlagsForRun: Optional[Callable[[int], Iterator[Optional[Sequence[Any]]]]] = None,
) -> Tuple[Dict[str, Any], List[float], List[Dict[str, Any]]]:
	"""
	algo_flags — на каждый прогон, если algoFlagsForRun не задан.
	algoFlagsForRun(iters) — функция из ноутбука: iters == n, внутри yield на прогон;
	вызывается как iter(algoFlagsForRun(n)). Если yield кончились раньше n — дальше algo_flags.
	weights — коэффициенты по ключам из METRIC_KEYS; 0 пропускается, знак задаёт направление оптимизации.
	"""
	g, a, _ = _parse(argv)
	out_dir = Path(out_dir or repo / "out" / "batch_runs")
	out_dir.mkdir(parents=True, exist_ok=True)
	it = iter(algoFlagsForRun(n)) if algoFlagsForRun is not None else None
	runs = []
	for i in range(n):
		out = out_dir / f"{g}_{a}_batch_{i}.json"
		av = list(argv)
		if "--output" in av:
			j = av.index("--output")
			av[j + 1] = str(out)
		else:
			av += ["--output", str(out)]
		af = algo_flags
		if it is not None:
			try:
				af = next(it)
			except StopIteration:
				af = algo_flags
		runs.append(runGd(av, repo, algo_flags=af))

	keys = [k for k in METRIC_KEYS if weights.get(k)]
	if not keys:
		keys = list(METRIC_KEYS)
	scores = [0.0] * n
	for k in keys:
		w = weights.get(k, 0.0)
		if not w: continue
		vals = []
		for r in runs:
			v = r["metrics"].get(k)
			vals.append(float(v) if v is not None else 0.0)
		for i, c in enumerate(_metricContribs(vals, w)):
			scores[i] += c
	best_i = max(range(n), key=lambda j: scores[j])
	return runs[best_i], scores, runs
