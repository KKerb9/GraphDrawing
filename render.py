import argparse
import json
import os

import matplotlib.pyplot as plt
import networkx as nx


def load_edges_from_dataset(graph_name: str, dataset_path: str) -> list[tuple[int, int]]:
	with open(dataset_path, "r", encoding="utf-8") as f:
		data = json.load(f)

	for item in data:
		if item.get("name") == graph_name:
			edges = item.get("edges", [])
			return [tuple(e) for e in edges]

	raise ValueError(f"graph '{graph_name}' not found in dataset '{dataset_path}'")


def build_graph(result: dict, dataset_path: str | None) -> tuple[nx.Graph, dict[int, tuple[float, float]]]:
	g = nx.Graph()

	nodes = result.get("nodes", [])
	if not nodes:
		raise ValueError("result JSON has no 'nodes' field")

	pos: dict[int, tuple[float, float]] = {}
	for node in nodes:
		node_id = node.get("id")
		x = node.get("x")
		y = node.get("y")
		if node_id is None or x is None or y is None:
			raise ValueError("each node must contain 'id', 'x' and 'y'")
		g.add_node(node_id)
		pos[node_id] = (x, y)

	edges = result.get("edges")
	if edges is None and dataset_path is not None:
		graph_name = result.get("graph_name")
		if graph_name is None:
			raise ValueError("result JSON has no 'graph_name' and no 'edges'; cannot restore edges")
		edges = load_edges_from_dataset(graph_name, dataset_path)

	if edges is not None:
		for u, v in edges:
			g.add_edge(u, v)

	return g, pos


def draw_graph(g: nx.Graph, pos: dict[int, tuple[float, float]], output_path: str, title: str | None = None) -> None:
	plt.figure(figsize=(6, 6))

	node_size = 170
	font_size = 7

	nx.draw_networkx_nodes(G=g, pos=pos, node_color="lightblue", edgecolors="black", node_size=node_size)
	nx.draw_networkx_edges(G=g, pos=pos, width=0.7)
	nx.draw_networkx_labels(G=g, pos=pos, labels={v: str(v) for v in g.nodes()}, font_size=font_size)

	if title:
		plt.title(title)

	plt.axis("equal")
	plt.axis("off")

	os.makedirs(os.path.dirname(output_path) or ".", exist_ok=True)
	plt.savefig(output_path, dpi=150, bbox_inches="tight")
	plt.close()


def parse_args() -> argparse.Namespace:
	parser = argparse.ArgumentParser(description="Render graph layout from artist JSON output.")
	parser.add_argument("input", help="path to JSON produced by 'artist'")
	parser.add_argument(
		"--dataset",
		default="samples/dataset.json",
		help="path to dataset.json with original graphs (used if result JSON has no 'edges')",
	)
	parser.add_argument(
		"--output",
		default=None,
		help="optional output PNG path; by default <dir>/<graph_name>_<algo>.png in the same directory as input JSON",
	)
	return parser.parse_args()


def main() -> int:
	args = parse_args()

	with open(args.input, "r", encoding="utf-8") as f:
		result = json.load(f)

	graph_name = result.get("graph_name", "graph")
	algo_name = result.get("algo", "algo")

	dataset_path = args.dataset if os.path.isfile(args.dataset) else None

	g, pos = build_graph(result, dataset_path)

	if args.output is not None:
		output_path = args.output
	else:
		base_dir = os.path.dirname(args.input)
		filename = f"{graph_name}_{algo_name}.png"
		output_path = os.path.join(base_dir if base_dir else ".", filename)

	title = f"{graph_name} ({algo_name})"
	draw_graph(g, pos, output_path, title)

	print(f"saved PNG to {output_path}")

	return 0


if __name__ == "__main__":
	raise SystemExit(main())

