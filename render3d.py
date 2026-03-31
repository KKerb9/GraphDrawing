"""
3D graph render for artist JSON with x, y, z per node (see EmbeddingWriter).

Usage (from project root):
  python render3d.py out/SmallGraph_random.json
  python render3d.py -o out/plot3d.png out/LargeGraph_random.json
"""

import argparse
import json
import os

import matplotlib.pyplot as plt
import networkx as nx


folder = "samples/"
main_json = "dataset.json"


def load_edges_from_dataset(graph_name, dataset_path):
        try:
                with open(dataset_path, "r", encoding="utf-8") as f:
                        data = json.load(f)
        except Exception as e:
                print("error while loading json" + str(e))
                raise ValueError("cannot load dataset for edges") from e

        for item in data:
                if item.get("name") == graph_name:
                        edges = item.get("edges", [])
                        return [tuple(e) for e in edges]

        raise ValueError(
                "graph '" + graph_name + "' not found in dataset '" + dataset_path + "'"
        )


def build_graph_3d(result, dataset_path):
        g = nx.Graph()

        nodes = result.get("nodes", [])
        if not nodes:
                raise ValueError("result JSON has no 'nodes' field")

        pos = {}
        for node in nodes:
                node_id = node.get("id")
                x = node.get("x")
                y = node.get("y")
                z = node.get("z")
                if node_id is None or x is None or y is None or z is None:
                        raise ValueError(
                                "each node must contain 'id', 'x', 'y' and 'z' (3D layout)"
                        )
                g.add_node(node_id)
                pos[node_id] = (float(x), float(y), float(z))

        edges = result.get("edges")
        if edges is None and dataset_path is not None:
                graph_name = result.get("graph_name")
                if graph_name is None:
                        raise ValueError(
                                "result JSON has no 'graph_name' and no 'edges'; cannot restore edges"
                        )
                edges = load_edges_from_dataset(graph_name, dataset_path)

        if edges is not None:
                skipped = 0
                for u, v in edges:
                        if u in pos and v in pos:
                                g.add_edge(u, v)
                        else:
                                skipped += 1
                if skipped:
                        print(
                                "warning: skipped "
                                + str(skipped)
                                + " edges (endpoint missing from layout nodes)"
                        )

        return g, pos


def draw_graph_3d(g, pos, output_path, title=None):
        fig = plt.figure(figsize=(7, 6))
        ax = fig.add_subplot(111, projection="3d")

        sz = 170
        font_sz = 7

        for u, v in g.edges():
                p0 = pos[u]
                p1 = pos[v]
                ax.plot(
                        [p0[0], p1[0]],
                        [p0[1], p1[1]],
                        [p0[2], p1[2]],
                        color="gray",
                        linewidth=0.7,
                )

        xs = [pos[n][0] for n in g.nodes()]
        ys = [pos[n][1] for n in g.nodes()]
        zs = [pos[n][2] for n in g.nodes()]
        ax.scatter(xs, ys, zs, c="lightblue", edgecolors="black", s=sz, depthshade=True)

        for n in g.nodes():
                p = pos[n]
                ax.text(p[0], p[1], p[2], str(n), fontsize=font_sz)

        if title:
                ax.set_title(title)

        ax.set_xlabel("x")
        ax.set_ylabel("y")
        ax.set_zlabel("z")

        try:
                ax.set_box_aspect(
                        [
                                max(xs) - min(xs) if len(xs) > 1 else 1.0,
                                max(ys) - min(ys) if len(ys) > 1 else 1.0,
                                max(zs) - min(zs) if len(zs) > 1 else 1.0,
                        ]
                )
        except AttributeError:
                pass

        out_dir = os.path.dirname(output_path) or "."
        os.makedirs(out_dir, exist_ok=True)
        plt.savefig(output_path, dpi=150, bbox_inches="tight")
        plt.close()


def parse_args():
        parser = argparse.ArgumentParser(
                description="Render 3D graph layout from artist JSON (x,y,z per node).",
                formatter_class=argparse.RawDescriptionHelpFormatter,
                epilog=(
                        "Examples:\n"
                        "  python render3d.py out/SmallGraph_random.json\n"
                        "  python render3d.py -o out/preview3d.png out/LargeGraph_random.json"
                ),
        )
        parser.add_argument(
                "input",
                nargs="?",
                default="out/SmallGraph_random.json",
                help="path to JSON from artist (default: %(default)s)",
        )
        parser.add_argument(
                "--dataset",
                default=folder + main_json,
                help="dataset.json path (only when result JSON has no \"edges\" field)",
        )
        parser.add_argument(
                "-o",
                "--output",
                default=None,
                metavar="PNG",
                help="output PNG (default: <dir of JSON>/<graph_name>_<algo>_3d.png)",
        )
        return parser.parse_args()


def main():
        args = parse_args()

        if not os.path.isfile(args.input):
                print("error: input file not found: " + args.input)
                print(
                        "hint: run from the project root, or pass the path to the JSON "
                        "(3D layout with x, y, z)."
                )
                return 1

        try:
                with open(args.input, "r", encoding="utf-8") as f:
                        result = json.load(f)
        except Exception as e:
                print("error while loading json" + str(e))
                return 1

        graph_name = result.get("graph_name", "graph")
        algo_name = result.get("algo", "algo")

        dataset_path = args.dataset
        if not os.path.isfile(dataset_path):
                print(
                        "warning: dataset file not found: "
                        + dataset_path
                        + " (edges only if present in result JSON)"
                )
                dataset_path = None

        try:
                g, pos = build_graph_3d(result, dataset_path)
        except ValueError as e:
                print("error: " + str(e))
                return 1

        if args.output is not None:
                output_path = args.output
        else:
                base_dir = os.path.dirname(args.input)
                filename = graph_name + "_" + algo_name + "_3d.png"
                output_path = os.path.join(base_dir if base_dir else ".", filename)

        title = graph_name + " (" + algo_name + ") — 3D"
        draw_graph_3d(g, pos, output_path, title)

        print("saved PNG to " + output_path)

        return 0


if __name__ == "__main__":
        raise SystemExit(main())
