import inspect
import json
import tempfile
import unittest
from pathlib import Path
from unittest.mock import patch

import matplotlib

matplotlib.use("Agg")

import numpy as np
import pandas as pd

import gd_experiments as gd


class GdExperimentsTest(unittest.TestCase):
	"""Проверки DataFrame-интерфейса экспериментов без файлов в репозитории."""

	def testFlagsToArgvAndStdinLine(self):
		"""Проверяет CLI-списки, совместимые maps и строки stdin."""
		flags = {"--graph": "graph1", "--3d": None, "--FS": "100,100"}
		self.assertEqual(gd.flagsToArgv(flags), ["--graph", "graph1", "--3d", "--FS", "100,100"])
		self.assertEqual(gd.flagsToArgv(["--graph", "graph1", "--3d", "--FS", "100,100"]), ["--graph", "graph1", "--3d", "--FS", "100,100"])
		self.assertEqual(gd.flagValue(["--graph", "graph1", "--seed", "10"], "--seed"), "10")
		self.assertEqual(gd.stdinLine({"--algo": "far"}, {"--C": 1.0, "--I": 100}, "algo"), "--C 1.0 --I 100\n")
		self.assertEqual(gd.stdinLine(["--algo", "far"], ["--C", "1.0", "--I", "100"], "algo"), "--C 1.0 --I 100\n")
		self.assertEqual(gd.stdinLine({"--algo": "far"}, None, "algo"), "\n")

	def testRunTestReturnsOneRowWithMaps(self):
		"""Проверяет однострочный DataFrame и сохранение maps запуска."""
		with tempfile.TemporaryDirectory() as tmp:
			repo = Path(tmp)
			jsonPath = repo / "result.json"
			jsonPath.write_text(json.dumps({"seed": 42, "metrics": {key: i for i, key in enumerate(gd.metricKeys)}}), encoding="utf-8")
			flags = {"--graph": "graph1", "--algo": "far", "--output": str(jsonPath)}
			with patch("gd_experiments.subprocess.run") as run:
				run.return_value.returncode = 0
				run.return_value.stdout = "done"
				run.return_value.stderr = ""
				df = gd.runTest(repo, flags, algoFlags={"--C": 1.0}, borderFlags={"--S": "100,100"})
			self.assertEqual(len(df), 1)
			self.assertEqual(df.at[0, "run"], 1)
			self.assertEqual(df.at[0, "seed"], 42)
			self.assertEqual(df.at[0, "flags"], flags)
			self.assertEqual(df.at[0, "algoFlags"], {"--C": 1.0})
			self.assertEqual(df.at[0, "minAngle"], 6)

	def testRunTestsAddsNumbersSeedsAndBatchOutputs(self):
		"""Проверяет номера запусков, seed и имена JSON пакетной серии."""
		calls = []

		def fakeRunTest(repo, flags, *, algoFlags=None, borderFlags=None):
			"""Подменяет запуск бинаря однострочным результатом без файлов."""
			calls.append((dict(flags), algoFlags, borderFlags))
			return pd.DataFrame([{key: 1.0 for key in gd.metricKeys} | {
				"run": 1,
				"flags": dict(flags),
				"algoFlags": algoFlags,
				"borderFlags": borderFlags,
			}])

		with tempfile.TemporaryDirectory() as tmp, patch("gd_experiments.runTest", side_effect=fakeRunTest):
			df = gd.runTests(
				Path(tmp),
				{"--graph": "graph1", "--algo": "far"},
				3,
				baseSeed=10,
				outDir=Path(tmp) / "results",
			)
		self.assertEqual(df["run"].tolist(), [1, 2, 3])
		self.assertEqual([call[0]["--seed"] for call in calls], [10, 11, 12])
		self.assertTrue(calls[0][0]["--output"].endswith("graph1_far_batch_0.json"))
		self.assertTrue(calls[2][0]["--output"].endswith("graph1_far_batch_2.json"))

	def testRunTestsSupportsMutableCliListsAndRangeCallbacks(self):
		"""Проверяет список токенов и callback, получающий range номеров запусков."""
		calls = []

		def fakeRunTest(repo, flags, *, algoFlags=None, borderFlags=None):
			"""Подменяет запуск и сохраняет независимые списки его аргументов."""
			calls.append((gd.copyFlags(flags), gd.copyFlags(algoFlags), gd.copyFlags(borderFlags)))
			return pd.DataFrame([{key: 1.0 for key in gd.metricKeys} | {
				"run": 1,
				"flags": gd.copyFlags(flags),
				"algoFlags": gd.copyFlags(algoFlags),
				"borderFlags": gd.copyFlags(borderFlags),
			}])

		flags = ["--graph", "base", "--algo", "far", "--output", "result.json", "--seed", "10"]
		borderFlags = ["--S", ""]

		def flagsForRun(indices):
			"""Меняет один общий список, как это делает metrics_from_dim.ipynb."""
			for index in indices:
				flags[1] = f"graph{index}"
				yield flags

		def borderFlagsForRun(indices):
			"""Создаёт размерности border policy из номера запуска."""
			for index in indices:
				borderFlags[1] = ",".join("5" for _ in range(index + 2))
				yield borderFlags

		with tempfile.TemporaryDirectory() as tmp, patch("gd_experiments.runTest", side_effect=fakeRunTest):
			df = gd.runTests(
				Path(tmp),
				flags,
				3,
				flagsForRun=flagsForRun,
				algoFlags=["--C", "1", "--I", "2"],
				borderFlags=borderFlags,
				borderFlagsForRun=borderFlagsForRun,
			)

		self.assertEqual(df["run"].tolist(), [1, 2, 3])
		self.assertEqual([gd.flagValue(call[0], "--graph") for call in calls], ["graph0", "graph1", "graph2"])
		self.assertEqual([gd.flagValue(call[0], "--seed") for call in calls], ["10", "11", "12"])
		self.assertEqual([call[2][1] for call in calls], ["5,5", "5,5,5", "5,5,5,5"])
		self.assertTrue(all(isinstance(value, list) for value in df["flags"]))

	def testScoreRunsAndBestRun(self):
		"""Проверяет положительные, отрицательные и константные вклады score."""
		df = pd.DataFrame({
			"run": [1, 2, 3],
			"volume": [1.0, 2.0, 3.0],
			"minVertexDist": [3.0, 2.0, 1.0],
			"maxVertexDist": [5.0, 5.0, 5.0],
		})
		for key in gd.metricKeys:
			if key not in df:
				df[key] = 0.0
		scored = gd.scoreRuns(df, {"volume": 2.0, "minVertexDist": -1.0, "maxVertexDist": 3.0})
		self.assertEqual(scored["score"].tolist(), [3.0, 4.5, 6.0])
		self.assertEqual(gd.bestRun(scored).at[2, "run"], 3)
		self.assertNotIn("score", df)

	def testSetImageScoreDoesNotWriteJson(self):
		"""Проверяет изменение только ячейки ручной оценки DataFrame."""
		df = pd.DataFrame({"run": [1], "imageScore": [0.0]})
		self.assertIs(gd.setImageScore(df, 1, 4.5), df)
		self.assertEqual(df.at[0, "imageScore"], 4.5)

	def testRenderRunReturnsPngPathWithoutChangingDf(self):
		"""Проверяет путь PNG renderRun и отсутствие изменений DataFrame."""
		with tempfile.TemporaryDirectory() as tmp:
			repo = Path(tmp)
			df = pd.DataFrame({"run": [1], "jsonPath": [str(repo / "result.json")], "pngPath": [None]})
			originalDf = df.copy(deep=True)
			with patch("gd_experiments.subprocess.run") as run:
				pngPath = gd.renderRun(repo, df, 1)
			self.assertEqual(pngPath, repo / "result.png")
			pd.testing.assert_frame_equal(df, originalDf)
			run.assert_called_once()

	def testDisplayTwoShowsBothImages(self):
		"""Проверяет два изображения, подписи и общий заголовок displayTwo."""
		with tempfile.TemporaryDirectory() as tmp:
			firstPath = Path(tmp) / "first.png"
			secondPath = Path(tmp) / "second.png"
			gd.plt.imsave(firstPath, np.zeros((8, 8, 3)))
			gd.plt.imsave(secondPath, np.ones((8, 8, 3)))
			fig, axes = gd.displayTwo(
				firstPath,
				secondPath,
				title1="Первый",
				title2="Второй",
				title="Сравнение",
			)
			self.assertEqual(len(axes), 2)
			self.assertEqual(axes[0].get_title(), "Первый")
			self.assertEqual(axes[1].get_title(), "Второй")
			self.assertEqual(fig._suptitle.get_text(), "Сравнение")
			gd.plt.close(fig)

	def testPlotMetricsPassesRawPointsToSeaborn(self):
		"""Проверяет line plot без агрегации и блок подписи серии."""
		df = pd.DataFrame({"run": [1, 2], **{key: [1.0, 2.0] for key in gd.metricKeys}})
		with patch("gd_experiments.sns.lineplot", wraps=gd.sns.lineplot) as lineplot:
			fig, ax = gd.plotMetrics(df, "volume", info={"Граф": "baseTreePath"})
			self.assertEqual(lineplot.call_args.kwargs["estimator"], None)
			self.assertEqual(lineplot.call_args.kwargs["errorbar"], None)
			self.assertEqual(len(ax.lines[0].get_xdata()), 2)
			self.assertIn("Граф: baseTreePath", [text.get_text() for text in ax.texts])
			fig.clf()
			fig, axes = gd.plotMetrics(df, "all", title="Серия", info="baseTreePath")
			self.assertEqual(axes.shape, (4, 3))
			self.assertEqual(lineplot.call_count, len(gd.metricKeys) + 1)
			self.assertIn("baseTreePath", [text.get_text() for text in fig.texts])
			fig.clf()

	def testPlotMetricsExplainsFailedRuns(self):
		"""Проверяет понятную ошибку вместо пустого графика из NaN-метрик."""
		df = pd.DataFrame({
			"run": [1, 2],
			"returnCode": [1, 1],
			"stderr": ["START\nError: invalid flags", "START\nError: invalid flags"],
			**{key: [float("nan"), float("nan")] for key in gd.metricKeys},
		})
		with self.assertRaisesRegex(ValueError, "все запуски завершились с ошибкой"):
			gd.plotMetrics(df)

	def testEveryModuleFunctionHasDocstring(self):
		"""Проверяет наличие docstring у каждой функции gd_experiments."""
		for _, func in inspect.getmembers(gd, inspect.isfunction):
			if func.__module__ == gd.__name__:
				self.assertTrue(inspect.getdoc(func), func.__name__)


if __name__ == "__main__":
	unittest.main()
