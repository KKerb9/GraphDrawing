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


def draw_graph(g, pos, output_path, title=None):
        plt.figure(figsize=(6, 6))

        sz = 170
        font_sz = 7

        nx.draw_networkx_nodes(
                G=g,
                pos=pos,
                node_color="lightblue",
                edgecolors="black",
                node_size=sz,
        )
        nx.draw_networkx_edges(G=g, pos=pos, width=0.7)
        nx.draw_networkx_labels(
                G=g,
                pos=pos,
                labels={v: str(v) for v in g.nodes()},
                font_size=font_sz,
        )

        if title:
                plt.title(title)

        plt.axis("equal")
        plt.axis("off")

        out_dir = os.path.dirname(output_path) or "."
        os.makedirs(out_dir, exist_ok=True)
        plt.savefig(output_path, dpi=150, bbox_inches="tight")
        plt.close()


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
                help="output PNG (default: <dir of JSON>/<graph_name>_<algo>.png)",
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
        except ValueError as e:
                print("error: " + str(e))
                return 1

        if args.output is not None:
                output_path = args.output
        else:
                base_dir = os.path.dirname(args.input)
                filename = graph_name + "_" + algo_name + ".png"
                output_path = os.path.join(base_dir if base_dir else ".", filename)

        title = graph_name + " (" + algo_name + ")"
        draw_graph(g, pos, output_path, title)

        print("saved PNG to " + output_path)

        return 0


if __name__ == "__main__":
        raise SystemExit(main())
