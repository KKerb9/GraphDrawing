# GraphDrawing

Для установки зависимостей выполнить:
```
pip install -r requirements.txt
```

Для работы с визуализатором:
1. Создать бинарь:
```
cmake --build build
```
2. Запустить с нужными параметрами:
```
./build/graph_drawing --help
```
3. Запустить питоновский скрипт для визуализации результата:
```
python3 render.py --help
```

## Итоговая асимптотика
Формула для итоговой асимптотики: $$\mathcal{O}(\text{Reader}) + \mathcal{O}(\text{InitialPlacement}) + \mathcal{O}(LayoutAlgoRun) + \mathcal{O}(Project) + \mathcal{O}(ComputeMetrics) + \mathcal{O}(\text{Writer})$$

- $\mathcal{O}(\text{Reader}) = \mathcal{O}(\text{Input}) + \mathcal{O}(\text{M})$
- $\mathcal{O}(\text{InitialPlacement}) = \begin{cases} 
\mathcal{O}(\text{N} \times \text{Dim}), \text{ random} \\
\mathcal{O}(\text{N} \times \text{Dim}), \text{ zero}
\end{cases}$
- $\mathcal{O}(\text{LayoutAlgoRun}) = \begin{cases}
\mathcal{O}(\text{N} \times \text{Dim}), \text{ random} \\
\mathcal{O}(\text{SpaceVolume}) + \mathcal{O}(\text{ITERS} \times \text{Dim} \times (\text{N}^2 + \text{M} + \text{N})) \text{ FaR}
\end{cases}$
- $\mathcal{O}(\text{SpaceVolume}) = \begin{cases}
\mathcal{O}(\text{Dim}), \text{ Euclidean} \\
\mathcal{O}(\text{Dim}^3), \text{ Lorentzian} \\
\mathcal{O}(1), \text{ Poincare} \\
\mathcal{O}(1), \text{ Klein}
\end{cases}$
- $\mathcal{O}(\text{Project}) = \begin{cases}
\mathcal{O}(\text{N} \times \text{FinalDim}), \text{ Ortogonal} \\
\mathcal{O}(\text{N} \times \text{Dim}), \text{ Identity} \\
\mathcal{O}(\text{N}), \text{ Poincare} \\
\mathcal{O}(\text{N} \times \text{Dim}), \text{ KleinOrtogonal} \\
\mathcal{O}(\text{C} \times (\text{N} \times \text{Dim} + (\text{N} + \text{M})^2)), \text{ KleinBestView} \\
\end{cases}$
- $\mathcal{O}(\text{ComputeMetrics}) = \mathcal{O}(\text{N} \times \text{FinalDim} + (\text{M} + \text{N}) ^ 2)$
- $\mathcal{O}(\text{Writer}) = \mathcal{O}(\text{N} + \text{M} + \text{FinalDim})$