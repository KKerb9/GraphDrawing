import json
import random
from collections import deque
from pathlib import Path


MAXN = 15  # 100

# нужны деревья с разным кол-вом вершин от 1 до MAXN
# с разным диаметром (от 1 до 500)
# еще одна характеристика -- 2 вершины с наибольшими степенями на
# различных расстояниях друг от друга (хранить и обе величины
# и расстояния между ними)
# так же считать медианное значение степеней вершин
# еще нужна доля листьев и доля вершин степени >= 3

DS_SIZE = 50000
SEED = 1543
OUTPUT = Path(__file__).resolve().parent.parent / "samples" / "TreeTreeBudetDirkaLeq15.json"


def make_tree(n, family, rng):
        edges = []
        params = {}
        if family == "path":
                edges = [(i - 1, i) for i in range(1, n)]
        elif family == "star":
                edges = [(0, i) for i in range(1, n)]
        elif family == "recursive":
                edges = [(rng.randrange(i), i) for i in range(1, n)]
        elif family == "binary":
                edges = [((i - 1) // 2, i) for i in range(1, n)]
        elif family == "r_ary":
                r = rng.randint(2, 12)
                params["branching"] = r
                edges = [((i - 1) // r, i) for i in range(1, n)]
        elif family == "double_hub":
                if n > 1:
                        k = rng.randint(1, n - 1)
                        params["hub_distance"] = k
                        share = rng.random()
                        params["first_hub_share"] = share
                        edges = [(i - 1, i) for i in range(1, k + 1)]
                        edges.extend((0 if rng.random() < share else k, i) for i in range(k + 1, n))
        elif family == "caterpillar":
                if n > 1:
                        diameter = rng.randint(2, n - 1) if n > 2 else 1
                        params["diameter"] = diameter
                        edges = [(i - 1, i) for i in range(1, diameter + 1)]
                        if diameter > 1:
                                hub = rng.randint(1, diameter - 1)
                                share = rng.random()
                                params["hub_share"] = share
                                edges.extend((hub if rng.random() < share else rng.randint(1, diameter - 1), i) for i in range(diameter + 1, n))
        else:
                # Случайное дерево с равномерным распределением размеченных деревьев.
                if n > 2:
                        import heapq
                        sequence = [rng.randrange(n) for _ in range(n - 2)]
                        degree = [1] * n
                        for v in sequence:
                                degree[v] += 1
                        leaves = [v for v in range(n) if degree[v] == 1]
                        heapq.heapify(leaves)
                        for v in sequence:
                                leaf = heapq.heappop(leaves)
                                edges.append((leaf, v))
                                degree[v] -= 1
                                if degree[v] == 1:
                                        heapq.heappush(leaves, v)
                        edges.append((heapq.heappop(leaves), heapq.heappop(leaves)))
                elif n == 2:
                        edges = [(0, 1)]
        order = list(range(n))
        rng.shuffle(order)
        return [(order[u], order[v]) for u, v in edges], params


def distances(adj, start):
        dist = [-1] * len(adj)
        dist[start] = 0
        queue = deque([start])
        while queue:
                v = queue.popleft()
                for u in adj[v]:
                        if dist[u] == -1:
                                dist[u] = dist[v] + 1
                                queue.append(u)
        return dist


def describe_tree(n, edges):
        adj = [[] for _ in range(n)]
        for u, v in edges:
                adj[u].append(v)
                adj[v].append(u)
        degrees = [len(neighbors) for neighbors in adj]
        first = distances(adj, 0)
        farthest = max(range(n), key=first.__getitem__)
        diameter = max(distances(adj, farthest))
        ordered = sorted(degrees)
        median = (ordered[(n - 1) // 2] + ordered[n // 2]) / 2
        maximum = max(degrees)
        candidates = [v for v in range(n) if degrees[v] == maximum]
        if n == 1:
                hubs = candidates
                top_distance = None
        elif len(candidates) == 1:
                first_hub = candidates[0]
                second_degree = max(degrees[v] for v in range(n) if v != first_hub)
                dist = distances(adj, first_hub)
                second_hub = max((v for v in range(n) if v != first_hub and degrees[v] == second_degree), key=lambda v: (dist[v], -v))
                hubs = [first_hub, second_hub]
                top_distance = dist[second_hub]
        else:
                dist = distances(adj, candidates[0])
                first_hub = max(candidates, key=lambda v: (dist[v], -v))
                dist = distances(adj, first_hub)
                second_hub = max(candidates, key=lambda v: (dist[v], -v))
                hubs = [first_hub, second_hub]
                top_distance = dist[second_hub]
        return {
                "n": n,
                "diameter": diameter,
                "top_vertices": hubs,
                "top_degrees": [degrees[v] for v in hubs],
                "top_distance": top_distance,
                "median_degree": median,
                "leaf_fraction": degrees.count(1) / n,
                "branch_fraction": sum(degree >= 3 for degree in degrees) / n,
        }


def generate_dataset():
        extremes = ("path", "star", "binary")
        families = ("recursive", "r_ary", "double_hub", "caterpillar", "prufer", "double_hub", "caterpillar", "double_hub", "caterpillar", "double_hub", "caterpillar", "double_hub", "caterpillar")
        rng = random.Random(SEED)
        weights = [n / sum(range(1, MAXN + 1)) * (DS_SIZE - MAXN) for n in range(1, MAXN + 1)]
        counts = [1 + int(weight) for weight in weights]
        remainder = DS_SIZE - sum(counts)
        for index in sorted(range(MAXN), key=lambda i: weights[i] - int(weights[i]), reverse=True)[:remainder]:
                counts[index] += 1
        sizes = [n for n, count in enumerate(counts, 1) for _ in range(count)]
        rng.shuffle(sizes)
        seen = [0] * (MAXN + 1)
        OUTPUT.parent.mkdir(parents=True, exist_ok=True)
        with OUTPUT.open("w", encoding="utf-8") as out:
                out.write("[\n")
                for i, n in enumerate(sizes):
                        round_index = seen[n]
                        seen[n] += 1
                        family = extremes[round_index] if round_index < len(extremes) else families[(round_index - len(extremes)) % len(families)]
                        seed = rng.randrange(2**32)
                        edges, params = make_tree(n, family, random.Random(seed))
                        item = {
                                "name": f"tree_{i + 1:06d}",
                                "description": f"{family} tree with {n} vertices",
                                "gen_algo": family,
                                "params": {"n": n, "seed": seed, **params},
                                "properties": describe_tree(n, edges),
                                "nodes": list(range(n)),
                                "edges": edges,
                        }
                        if i:
                                out.write(",\n")
                        json.dump(item, out, separators=(",", ":"))
                out.write("\n]\n")


if __name__ == "__main__":
        generate_dataset()
