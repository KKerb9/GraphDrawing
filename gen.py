import os

import json

import networkx as nx
from networkx.readwrite import json_graph
import matplotlib.pyplot as plt


folder = "samples/"
main_json = "dataset.json"

data = []


def load_json():
        global data
        try:
                with open(folder + main_json, "r", encoding="utf-8") as f:
                        data = json.load(f)
        except Exception as e:
                print("error while loading json" + str(e))
                data = []


def upload_json():
        with open(folder + main_json, "w", encoding="utf-8") as f:
                json.dump(data, f, ensure_ascii=False, indent=4)


def gen1(name: str="ERgraph", n: int=50, p: float=0.1):
        g = nx.gnp_random_graph(n, p)

        l = nx.spring_layout(g)
        sz = 170
        font_sz = 7
        desc = "default undirected graph"
        nx.draw_networkx_nodes(G=g, pos=l, node_color='lightblue', edgecolors='black', node_size=sz)
        nx.draw_networkx_edges(G=g, pos=l, width=0.7)
        nx.draw_networkx_labels(G=g, pos=l, labels={v: str(v) for v in g.nodes()}, font_size=font_sz)
        # nx.draw_networkx(G=g, pos=l, node_color='', with_labels=True, node_size=sz)
        plt.savefig(folder + name + ".png", dpi=150)
        plt.close()

        new_data = {
                "name"        : name,
                "description" : desc,
                "gen_algo"    : "nx.gnp_random_graph",
                "params"      : {"n" : n, "p" : p},
                "nodes"       : list(g.nodes()),
                "edges"       : [list(e) for e in g.edges()]
        }
        data.append(new_data)


def gen2(name: str, szs: list=[10, 10, 10, 10, 10], P: list[list]=None): 
        if P is None:
                P = [] * len(szs)
                for i in range(len(szs)):
                        P[i][i] = 0.1
        g = nx.stochastic_block_model(sizes=szs, p=P, seed=0)  # seed что бы фикс. генератор был

        l = nx.spring_layout(g)
        sz = 170
        font_sz = 7
        desc = "sbm undirected graph"
        nx.draw_networkx_nodes(G=g, pos=l, node_color='lightblue', edgecolors='black', node_size=sz)
        nx.draw_networkx_edges(G=g, pos=l, width=0.7)
        nx.draw_networkx_labels(G=g, pos=l, labels={v: str(v) for v in g.nodes()}, font_size=font_sz)
        plt.savefig(folder + name + ".png", dpi=150)
        plt.close()

        new_data = {
                "name"        : name,
                "description" : desc,
                "gen_algo"    : "nx.stochastic_block_model",
                "params"      : {"sizes" : szs, "p" : P, "seed" : 0},
                "nodes"       : list(g.nodes()),
                "edges"       : [list(e) for e in g.edges()]
        }
        data.append(new_data)


if __name__ == "__main__":
        load_json()

        # for i in range(1, 11):
        #         gen1(name=("graph" + str(i)), n=50, p=(0.1 * i))

        # szs = [10, 10, 10, 10, 10]
        # P = [
        #         [0.2, 0.001, 0.001, 0.001, 0.001],
        #         [0.001, 0.2, 0.001, 0.001, 0.001],
        #         [0.001, 0.001, 0.2, 0.001, 0.001],
        #         [0.001, 0.001, 0.001, 0.2, 0.001],
        #         [0.001, 0.001, 0.001, 0.001, 0.2]
        # ]
        # for i in range(1, 11):
        #         for j in range(5):
        #                 P[j][j] = 0.1 * i
        #         gen2(name="SBMgraph" + str(i), szs=szs, P=P)

        # szs = [10, 10, 10, 10, 10]
        # P = [
        #         [0.2, 0.05, 0.05, 0.05, 0.05],
        #         [0.05, 0.2, 0.05, 0.05, 0.05],
        #         [0.05, 0.05, 0.2, 0.05, 0.05],
        #         [0.05, 0.05, 0.05, 0.2, 0.05],
        #         [0.05, 0.05, 0.05, 0.05, 0.2]
        # ]
        # for i in range(1, 11):
        #         for j in range(5):
        #                 P[j][j] = 0.1 * i
        #         gen2(name="SBMgraph" + str(10 + i), szs=szs, P=P)

        # gen1("LargeGraph", n=100, p=0.5)
        
        gen1("SmallGraph", n=5, p=0.7)

        upload_json()


