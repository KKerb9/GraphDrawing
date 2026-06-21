import argparse
import json
import math
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


def build_graph(result, dataset_path):
        g = nx.Graph()

        nodes = result.get("nodes", [])
        if not nodes:
                raise ValueError("result JSON has no 'nodes' field")

        pos = {}
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


def get_fig_size(result, dim):
        fig_size = result.get("fig_size")
        if fig_size is None:
                return None
        if len(fig_size) != dim:
                raise ValueError("fig_size length must be " + str(dim))
        return [float(x) for x in fig_size]


def poincare_geodesic(a, b, radius, samples=80):
        ax, ay = a
        bx, by = b
        determinant = ax * by - ay * bx
        scale = max(radius * radius, 1.0)
        if abs(determinant) <= 1e-12 * scale:
                return [ax, bx], [ay, by]

        aValue = (ax * ax + ay * ay + radius * radius) / 2.0
        bValue = (bx * bx + by * by + radius * radius) / 2.0
        centerX = (aValue * by - ay * bValue) / determinant
        centerY = (ax * bValue - aValue * bx) / determinant
        circleRadius = math.sqrt(max(centerX * centerX + centerY * centerY - radius * radius, 0.0))

        start = math.atan2(ay - centerY, ax - centerX)
        end = math.atan2(by - centerY, bx - centerX)
        shortSweep = (end - start + math.pi) % (2.0 * math.pi) - math.pi
        middle = start + shortSweep / 2.0
        middleX = centerX + circleRadius * math.cos(middle)
        middleY = centerY + circleRadius * math.sin(middle)
        if middleX * middleX + middleY * middleY > radius * radius:
                shortSweep -= math.copysign(2.0 * math.pi, shortSweep)

        angles = [start + shortSweep * i / (samples - 1) for i in range(samples)]
        return (
                [centerX + circleRadius * math.cos(angle) for angle in angles],
                [centerY + circleRadius * math.sin(angle) for angle in angles],
        )


def draw_graph(g, pos, output_path, title=None, fig_size=None, drawing_space=None):
        fig, ax = plt.subplots(figsize=(8, 8))
        fig.patch.set_facecolor("#f7f9fc")
        ax.set_facecolor("#f7f9fc")

        sz = 170
        font_sz = 7

        nx.draw_networkx_nodes(
                G=g,
                pos=pos,
                ax=ax,
                node_color="#8ecae6",
                edgecolors="#24445c",
                linewidths=1.1,
                node_size=sz,
        )
        if drawing_space == "poincare":
                if fig_size is None:
                        raise ValueError("Poincare rendering requires fig_size")
                radius = min(fig_size) / 2.0
                boundary = plt.Circle(
                        (0.0, 0.0),
                        radius,
                        fill=False,
                        color="#667085",
                        linewidth=1.1,
                )
                ax.add_patch(boundary)
                for u, v in g.edges():
                        xs, ys = poincare_geodesic(pos[u], pos[v], radius)
                        ax.plot(xs, ys, color="#475467", linewidth=0.85, alpha=0.8, zorder=1)
        else:
                nx.draw_networkx_edges(
                        G=g,
                        pos=pos,
                        ax=ax,
                        width=0.85,
                        edge_color="#475467",
                        alpha=0.8,
                )
        nx.draw_networkx_labels(
                G=g,
                pos=pos,
                ax=ax,
                labels={v: str(v) for v in g.nodes()},
                font_size=font_sz,
                font_color="#172b3a",
        )

        if title:
                ax.set_title(title, fontsize=15, pad=14)

        ax.set_aspect("equal", adjustable="box")
        if fig_size is not None:
                ax.set_xlim(-fig_size[0] / 2.0, fig_size[0] / 2.0)
                ax.set_ylim(-fig_size[1] / 2.0, fig_size[1] / 2.0)
        ax.axis("off")

        out_dir = os.path.dirname(output_path) or "."
        os.makedirs(out_dir, exist_ok=True)
        fig.tight_layout(pad=0.8)
        fig.savefig(output_path, dpi=180, bbox_inches="tight", facecolor=fig.get_facecolor())
        plt.close(fig)


def parse_args():
        parser = argparse.ArgumentParser(
                description="Render graph layout from artist JSON output."
        )
        parser.add_argument("input", help="path to JSON produced by 'artist'")
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
                help='output PNG (default: same directory as JSON, name "<input_basename>.png")',
        )
        return parser.parse_args()


def main():
        args = parse_args()

        if not os.path.isfile(args.input):
                print("error: input file not found: " + args.input)
                print(
                        "hint: run from the project root, or pass the full path to the JSON "
                        "(e.g. out/SmallGraph_random.json)."
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
                g, pos = build_graph(result, dataset_path)
                fig_size = get_fig_size(result, 2)
        except ValueError as e:
                print("error: " + str(e))
                return 1

        if args.output is not None:
                output_path = args.output
        else:
                base_dir = os.path.dirname(args.input)
                stem = os.path.splitext(os.path.basename(args.input))[0]
                filename = stem + ".png"
                output_path = os.path.join(base_dir if base_dir else ".", filename)

        title = graph_name + " (" + algo_name + ")"
        drawing_space = result.get("drawing_space", "euclidean")
        draw_graph(g, pos, output_path, title, fig_size, drawing_space)

        print("saved PNG to " + output_path)

        return 0


if __name__ == "__main__":
        raise SystemExit(main())
