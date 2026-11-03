# Theoretical Approach of the Graph Library in C

This document summarizes the theoretical foundations, algorithmic design, and data structure choices implemented in the C graph library (`Trabalho 1 (Implementação em C)`).

---

## 1. Core Representations and Graph Invariants

The library models **undirected, simple graphs** $G = (V, E)$, with $n = |V|$ vertices and $m = |E|$ edges. Vertices in files and public APIs are $1$-based ($1, \dots, n$), mapped internally to $0$-based indices ($0, \dots, n - 1$). Part 1 assumes unweighted edges; Part 2 extends every edge with a weight $w(u,v) \in \mathbb{R}$ (possibly negative at read time — Dijkstra rejects negative inputs).

The `Graph` struct uses a union to support two alternative internal representations:

```c
typedef unsigned char MatrixCell;

typedef struct Graph {
    int n;
    bool adj_list;
    bool weighted;        /* true when the input file has a weight != 1 */
    bool negative_weight; /* true when any edge has weight < 0          */
    double **weights;     /* Matrix edge weights, allocated lazily      */
    union {
        Node **list;         /* Adjacency list: array of n singly-linked lists */
        MatrixCell **matrix; /* Adjacency matrix: n x n byte grid              */
    } graph;
} Graph;
```

### Ingestion and Simple Graph Guarantees
- **Self-loops** ($(u, u)$): Ignored during file reading to enforce that $G$ is simple.
- **Duplicate edges** (multiple occurrences of $(u, v)$): Ignored. In the matrix, setting cell values to `1` is idempotent. In the list, neighbor lists are sorted ascendingly during ingestion and consecutive duplicate elements are discarded.
- **Ordered Neighbors**: Neighbors are maintained in strictly **ascending order** in both representations, guaranteeing deterministic and identical traversal orders between the matrix and the list.

---

## 2. Feature-by-Feature Theoretical Approaches

### 2.1. Vertex Degree ($d(v)$)
The degree $d(v)$ represents the number of edges incident to vertex $v$.

- **Adjacency List**:
  - **Approach**: Traverses the singly linked list of vertex $v$ starting from `graph.list[v]->next` (skipping the dummy sentinel head) and **counts the number of pointer steps** until reaching `NULL`.
  - **Time Complexity**: $\Theta(d(v))$.
- **Adjacency Matrix**:
  - **Approach**: Iterates through row $v$ across all columns $u \in \{0, \dots, n-1\}$ and **sums the entries** (or counts the number of cells where $M[v][u] == 1$).
  - **Time Complexity**: $\Theta(n)$.

```c
int degree(Graph *g, int v) {
    int d = 0, u;
    NeighborIter it = neighbor_iter(g, v);
    while (neighbor_next(&it, &u, NULL)) d++;
    return d;
}
```

---

### 2.2. Total Edge Count ($m$) and Degree Statistics
- **Handshaking Lemma (Lema do Aperto de Mão)**:
  Every undirected edge has two endpoints, so the sum of degrees over all vertices equals twice the total number of edges:
  $$\sum_{v \in V} d(v) = 2m \implies m = \left\lfloor \frac{1}{2} \sum_{v \in V} d(v) \right\rfloor$$
  `edge_count(g)` computes all degrees, sums them, and performs integer division by 2.
- **Degree Statistics**:
  - **Minimum Degree**: $\delta(G) = \min_{v \in V} d(v)$ in $\Theta(n)$.
  - **Maximum Degree**: $\Delta(G) = \max_{v \in V} d(v)$ in $\Theta(n)$.
  - **Mean Degree**: $\bar{d} = \frac{1}{n} \sum_{v \in V} d(v) = \frac{2m}{n}$.
  - **Median Degree**: The degree vector is sorted in ascending order via `qsort` ($O(n \log n)$). If $n$ is odd, the median is the center value; if $n$ is even, it is the arithmetic average of the two middle elements.

---

### 2.3. Zero-Allocation Neighborhood Iterator (`NeighborIter`)
Provides a unified `static inline` gateway (`neighbor_iter(g, v)` / `neighbor_next(&it, &u, &w)`) over both representations without heap allocation:
- **Adjacency List**: Steps through `Node *next` in $\Theta(d(v))$ total time, reading `node->data` and `node->weight`.
- **Adjacency Matrix**: Scans row $v$ in $\Theta(n)$ total time, yielding indices where $M[v][u] == 1$ and reading `g->weights[v][u]` (or $1.0$ if unweighted).

---

### 2.4. Breadth-First Search (BFS)
- **Theoretical Basis**: Level-synchronous exploration using a FIFO queue (`Queue`). Explores vertices in non-decreasing order of their shortest-path distance from the source.
- **State Maintained**:
  - `vector`: Boolean array tracking whether a vertex has been discovered.
  - `parents`: Records the predecessor in the BFS tree (initialized to `-1`).
  - `levels`: Distance from the root ($0$ at root, `-1` if unreachable).
  - `result_discovered`: Sequence of vertices in order of queue departure.
- **Early-Exit**: An optional `target` parameter terminates the search immediately upon dequeuing `target`.
- **Complexity**:
  - List: $O(n + m)$ time.
  - Matrix: $O(n^2)$ time.

---

### 2.5. Depth-First Search (DFS)
- **Theoretical Basis**: Deep exploration via backtracking using an explicit LIFO stack (`Stack`).
- **Symmetry & Determinism with Matrix**:
  To visit smaller-numbered neighbors first (matching the natural matrix loop $0 \dots n-1$):
  - In **Adjacency Matrix**: The row is traversed backwards ($n-1$ down to $0$) when pushing onto the stack, so that the smallest index ends up at the top of the stack.
  - In **Adjacency List**: Neighbors are gathered into a temporary dynamic array and pushed in reverse order (last to first), ensuring the smallest neighbor is popped first.
- **Complexity**:
  - List: $O(n + m)$ time.
  - Matrix: $O(n^2)$ time.

---

### 2.6. Shortest Path Distance (`Graph_distance(u, v)`)
- In an unweighted graph, the shortest path between $u$ and $v$ is the minimum number of edges connecting them, which corresponds to the BFS tree level of $v$ rooted at $u$.
- `Graph_distance` executes a BFS from $u$ and returns `levels[v]`. If $u$ and $v$ belong to different connected components, it returns `-1` (infinite distance).

---

### 2.7. Graph Diameter (`Graph_diameter`)
The diameter of a connected graph is the maximum shortest-path distance between any pair:
$$\mathrm{diam}(G) = \max_{u, v \in V} d(u, v) = \max_{u \in V} \epsilon(u)$$
where $\epsilon(u) = \max_{v \in C(u)} d(u, v)$ is the eccentricity of $u$. For disconnected graphs, it is the maximum diameter among all components.

1. **Exact Diameter** (`approximate = 0`):
   - Runs a BFS from **every** vertex $u \in V$ ($n$ full BFS runs).
   - Computes $\epsilon(u)$ and tracks the global maximum.
   - Time Complexity: $O(n(n + m))$ for list, $O(n^3)$ for matrix. Feasible only for small-to-moderate graphs ($n \le 60{,}000$).
2. **Approximated Diameter (2-Sweep Heuristic)** (`approximate = 1`):
   - Designed for massive graphs where $O(n^2)$ runs are intractable.
   - For each connected component:
     1. Pick an arbitrary seed vertex $s$ (e.g., the first vertex).
     2. Run BFS from $s$ to find the farthest vertex $u = \arg\max_x d(s, x)$.
     3. Run a second BFS from $u$ to find the farthest vertex $w = \arg\max_x d(u, x)$.
     4. The distance $d(u, w)$ is a lower-bound estimate: $d(u, w) \le \mathrm{diam}(C) \le 2 \cdot d(u, w)$ (exact on trees).
   - Time Complexity: Only 2 BFS executions per component $\implies O(n + m)$ total time.

---

### 2.8. Connected Components (`Graph_connected_components`)
- **Theoretical Basis**: Equivalence classes of the reachability relation.
- **Approach**:
  - Maintains a global `visited` boolean array.
  - Iterates $v$ from $0$ to $n-1$; if $v$ is unvisited, initiates a BFS from $v$. The set of all vertices reached in this BFS constitutes exactly one connected component.
  - Each component's vertices are sorted in ascending order.
  - The list of components is sorted in **descending order of size** using **Insertion Sort** (chosen specifically because it is a stable sort, preserving discovery order for ties, matching Python's reference behavior).
- **Time Complexity**: $O(n + m)$ for list, $O(n^2)$ for matrix.

### 2.9. Weighted Graphs (Part 2)
- **File Format**: each edge line carries an optional third column, the weight (any floating-point value, negative included). Without the third column the edge costs $1$, so every Part 1 file is still valid and behaves identically.
- **Storage**:
  - *Adjacency list*: the weight lives in the edge node itself (`Node.weight`) — no extra lookup, $\Theta(1)$ per edge visit.
  - *Adjacency matrix*: the $1$-byte `MatrixCell` grid still stores adjacency only; weights live in a separate `double**` matrix allocated **lazily**, only when the file actually contains a weight $\neq 1$ (an $n^2$ `double` array would dwarf the adjacency matrix itself). Since it is born at the *first* weighted edge, edges read earlier are backfilled with $1.0$. A legitimate weight of $0.0$ does not alias "no edge", because weights are only read for cells that carry an edge.
  - *Access*: `neighbor_next` yields the pair $(u, w)$ — the same single gateway of Part 1, so no algorithm knows where weights live.
- **Duplicates with weights**: between repeated lines for the same edge, the **last** one wins in both representations (the matrix overwrites naturally; the list sorts by $(vertex, file\ order)$ and keeps the last of each run).
- **Negative-weight flag**: detected during ingestion (`Graph.negative_weight`), so Dijkstra rejects in $O(1)$.

### 2.10. Dijkstra's Algorithm (Part 2)
- **Theoretical basis**: with **non-negative** weights, the greedy invariant holds — when $u$ is extracted with the smallest tentative distance, $dist[u]$ is final. Relaxation: $dist[v] \leftarrow \min(dist[v], dist[u] + w(u,v))$.
- **One algorithm, two estimate stores** (the assignment explicitly asks not to duplicate the algorithm): `dijkstra()` is written once over the `DistStore` abstraction — *how estimates are stored, updated, and how the minimum is extracted*. Two implementations plug into it:
  1. **Vector store** — every vertex starts *active* with estimate $+\infty$; `extract_min` scans for the smallest finite active estimate. Cost: $n$ extractions × $\Theta(n)$ scan → $\Theta(n^2)$, plus relaxation $O(m)$: total $O(n^2 + m)$ (matrix: $O(n^2)$ row scans dominate).
  2. **Heap store** — only vertices with a finite estimate enter the `MinHeap` (lazy insertion on first relaxation); an update of an already-present vertex is `MinHeap_decrease_key`, which locates the element in $O(1)$ via the `posicao[vertice]` back-array and sifts up in $O(\log n)$ — the *efficient key modification* the assignment demands. Total: $O((n + m)\log n)$ (list); the matrix's row scans dominate to $O(n^2)$.
- **Unreachable vertices**: they never acquire a finite estimate, stay at $\infty$ ($+\infty$ = `HUGE_VAL`; Part 1's $-1$ sentinel is a legal distance here), and the loop ends when the store reports no finite estimate left.
- **Path reconstruction**: `parents[]` records the relaxing predecessor; `dijkstra_path` walks it back from the target and reverses in place.
- **Negative weights**: rejected with an informative message (return `NULL`) — shortest paths with negative weights are out of scope for this part.

---



## 3. Complexity Comparison Summary

| Feature / Operation | Adjacency List | Adjacency Matrix | Notes |
|---|---|---|---|
| **Space Complexity** | $\Theta(n + m)$ | $\Theta(n^2)$ | Matrix uses 1-byte `unsigned char` cells |
| **Edge Existence $(u, v) \in E$** | $O(d(u))$ | $\Theta(1)$ | List traverses linked list |
| **Vertex Degree $d(v)$** | $\Theta(d(v))$ (pointer steps) | $\Theta(n)$ (sums/counts 1s in row) | Direct answer to the user query |
| **All Degrees / Edge Count** | $\Theta(n + m)$ | $\Theta(n^2)$ | Handshaking Lemma: $m = \frac{1}{2}\sum d(v)$ |
| **BFS / DFS** | $\Theta(n + m)$ | $\Theta(n^2)$ | Ascending tie-breaking guarantees identical trees |
| **Distance $d(u, v)$** | $O(n + m)$ | $O(n^2)$ | Single BFS run |
| **Exact Diameter** | $O(n(n + m))$ | $O(n^3)$ | $n$ full BFS runs (All-Pairs) |
| **Approximate Diameter** | $O(n + m)$ | $O(n^2)$ | Double-sweep BFS (2 BFS runs per component) |
| **Connected Components** | $\Theta(n + m)$ | $\Theta(n^2)$ | Full graph sweep with BFS + stable sort |
| **Edge weight $w(u,v)$ (Part 2)** | $\Theta(1)$ (field in node) | $\Theta(1)$ (weights matrix, lazy) | Delivered by `neighbor_next` |
| **Dijkstra (vector store)** | $O(n^2 + m)$ | $O(n^2)$ | Linear-scan `extract_min` |
| **Dijkstra (heap store)** | $O((n + m)\log n)$ | $O(n^2)$ | `decrease-key` via `posicao[]`; matrix row scans dominate |

---

## 4. Key C Engineering Decisions

1. **Compact 1-Byte Matrix Cells (`MatrixCell`)**:
   Instead of 4-byte `int`, matrix cells are `unsigned char` ($0$ or $1$). For $n \approx 50{,}000$ (`grafo_2`), this reduced RAM usage from $\sim 9.3\text{ GB}$ to $\sim 2.3\text{ GB}$, fitting within physical memory.
2. **Sentinel Dummy Head Nodes**:
   Each adjacency list starts with a dummy head node `Node(-1, NULL)`. Insertions and deletions never require branching on whether the head pointer itself is `NULL`.
3. **Fast Buffered I/O (`Leitor`)**:
   Input files can exceed 700 MB (`grafo_6.txt`). Using a 64 KB block buffer (`fread`) with manual ASCII-to-integer conversion reduced loading time from minutes to under a second.
4. **Linear String Accumulator (`StrBuf`)**:
   Generating output files for massive graphs involves millions of formatting operations. `StrBuf` implements exponential buffer doubling ($O(1)$ amortized append), avoiding the $O(n^2)$ overhead of repeated `strlen` and `realloc` with `str_append`.
5. **Two-Pass Single-Block Adjacency List (`alocar_list_graph`)**:
   Instead of calling `malloc` per edge node (which adds 8 bytes of allocator metadata per 24-byte `Node` plus temporary per-vertex edge vectors), pass 1 counts vertex degrees and allocates a single contiguous block of `n + 2m` `Node`s anchored at `graph[0]`, and pass 2 writes sorted neighbors in place. This reduces peak RAM by ~35–39% and allows `grafo_W_7` (50M vertices, 60M edges) to run in 5.0 GB RAM without swap.
