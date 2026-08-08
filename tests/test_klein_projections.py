import json
import math
import os
import subprocess
import sys
import tempfile
import unittest
from pathlib import Path


class KleinProjectionTest(unittest.TestCase):
	"""Интеграционные проверки Klein-проекций во временной сборке."""

	@classmethod
	def setUpClass(cls):
		"""Собирает бинарь в отдельном временном каталоге для всех проверок."""
		cls.repo = Path(__file__).resolve().parents[1]
		cls.tmp = tempfile.TemporaryDirectory()
		cls.root = Path(cls.tmp.name)
		cls.buildDir = cls.root / "build"
		cmakeArgs = ["cmake", "-S", str(cls.repo), "-B", str(cls.buildDir)]
		cachePath = cls.repo / "build" / "_deps" / "nlohmann_json-install" / "share" / "cmake" / "nlohmann_json"
		if cachePath.is_dir():
			cmakeArgs.append("-Dnlohmann_json_DIR=" + str(cachePath))
		subprocess.run(cmakeArgs, check=True, capture_output=True, text=True)
		subprocess.run(["cmake", "--build", str(cls.buildDir)], check=True, capture_output=True, text=True)
		cls.binary = cls.buildDir / "graph_drawing"

	@classmethod
	def tearDownClass(cls):
		"""Удаляет временную сборку, JSON, PNG и Matplotlib-кэш тестов."""
		cls.tmp.cleanup()

	def runGd(self, projection, output, *, dim=3, candidates=32):
		"""Запускает Hn-layout с Klein-проекцией и возвращает JSON результата."""
		command = [
			str(self.binary),
			"--graph", "SmallGraph",
			"--algo", "random",
			"--space", "hyperbolic",
			"--initial-placement", "random",
			"--dim", str(dim),
			"--2d",
			"--projection", projection,
			"--cameraCandidates", str(candidates),
			"--FS", "200,200",
			"--seed", "123",
			"--output", str(output),
		]
		process = subprocess.run(command, input="\n", check=False, capture_output=True, text=True)
		self.assertEqual(process.returncode, 0, process.stderr)
		with open(output, encoding="utf-8") as f:
			return json.load(f)

	def assertKleinDisk(self, result):
		"""Проверяет имя drawing space и принадлежность всех вершин диску Klein."""
		self.assertEqual(result["drawing_space"], "klein")
		radius = min(result["fig_size"]) / 2.0
		for node in result["nodes"]:
			self.assertLess(math.hypot(node["x"], node["y"]), radius)

	def testProjectsH3AndH4IntoKleinDisk(self):
		"""Проверяет обе проекции на H3 и H4 без выхода за границу диска."""
		orthogonal = self.runGd("kleinOrthogonal", self.root / "orthogonal.json", dim=3)
		bestView = self.runGd("kleinBestView", self.root / "best.json", dim=4)
		self.assertKleinDisk(orthogonal)
		self.assertKleinDisk(bestView)

	def testBestViewIsDeterministicAndContainsOrthogonalCandidate(self):
		"""Проверяет детерминизм камеры и базовый кандидат при бюджете один."""
		first = self.runGd("kleinBestView", self.root / "first.json", candidates=32)
		second = self.runGd("kleinBestView", self.root / "second.json", candidates=32)
		orthogonal = self.runGd("kleinOrthogonal", self.root / "orthogonal.json", candidates=1)
		bestOne = self.runGd("kleinBestView", self.root / "best-one.json", candidates=1)
		self.assertEqual(first["nodes"], second["nodes"])
		self.assertEqual(orthogonal["nodes"], bestOne["nodes"])

	def testRejectsInvalidCameraCandidates(self):
		"""Проверяет раннюю CLI-ошибку при неположительном бюджете камер."""
		command = [
			str(self.binary), "--space", "hyperbolic", "--dim", "3", "--2d",
			"--projection", "kleinBestView", "--cameraCandidates", "0",
		]
		process = subprocess.run(command, input="\n", check=False, capture_output=True, text=True)
		self.assertNotEqual(process.returncode, 0)
		self.assertIn("--cameraCandidates must be positive", process.stderr)

	def testRenderKleinOutput(self):
		"""Проверяет создание PNG рендерером для прямолинейного Klein-рисунка."""
		jsonPath = self.root / "render.json"
		self.runGd("kleinOrthogonal", jsonPath)
		pngPath = self.root / "render.png"
		env = dict(os.environ, MPLCONFIGDIR=str(self.root / "mpl"))
		process = subprocess.run(
			[sys.executable, str(self.repo / "render.py"), str(jsonPath), "-o", str(pngPath)],
			check=False,
			capture_output=True,
			text=True,
			env=env,
		)
		self.assertEqual(process.returncode, 0, process.stderr)
		self.assertTrue(pngPath.is_file())


if __name__ == "__main__":
	unittest.main()
