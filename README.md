# Compiler Register Allocation Algorithms

FEUP LEIC016 / DA — *Spring 2026, Programming Project II*

A C++17 tool that reads variable live-range descriptions and a register
budget, builds the interference graph, and tries to color it. Four algorithm
variants are supported (matching the project spec):

| spec value (`algorithm:` in config) | description |
| --- | --- |
| `basic`                | Chaitin simplify-and-pop, no spilling. Reports infeasible if N registers don't suffice. |
| `spilling, K`          | Same, but allowed to spill up to K webs to memory. |
| `splitting, K`         | Same, but allowed to split up to K webs at their definition points. |
| `free`                 | Our own heuristic — DSatur (Brélaz, 1979) coloring. |

The interference graph is built on top of the lecture-style templated
`Graph<T>` in [`src/Graph.h`](src/Graph.h) with two minor additions used by
the coloring algorithms (a per-vertex `active` flag for logical removal and a
`color` slot).

## Build

Either of:

```bash
make                              # plain Makefile (default; puts artifacts in build/)
# or
cmake -S . -B build && cmake --build build   # CMake equivalent
```

Both produce `build/regalloc`.

## Use

### Interactive menu

```bash
./build/regalloc
```

Walks through: load ranges → load config → build webs → show interference
graph → run allocation → write output.

### Batch (script-friendly)

```bash
./build/regalloc -b ranges.txt registers.txt allocation.txt
```

Exit codes: `0` feasible, `1` infeasible (output still produced, all webs as
`M:`), `2` parse / I/O error.

## Input file formats

### Live-range file

```
# comments and blank lines are ignored
sum: 7+,8,9,10-
i:   1+,2,3,4,5,6-
i:   9+,10,11,12-
i:  12+,13,14-
i:  20+,11,12-
```

Each line is one *live range*. A `+` after a line number marks the
definition that starts that range; a `-` marks the last use that ends it.
Multiple ranges of the same variable that share a program line get merged
into a single *web*. When two ranges of the same variable share a line where
one has `+` and the other `-` (the `i = i + 1` pattern), the markers cancel
and the line becomes a flow-through point of the merged web.

### Config file

```
registers: 3
algorithm: spilling, 2   # or: basic | splitting, K | free
```

## Output format

Matches the spec's Figures 11 / 12:

```
# Total number of webs followed by the listing of the program points of each one
# program points in each web are sorted in ascending order
webs: 3
web0: 1+,2,3,4,5,6-
web1: 9+,10,11,12,13,14-,20+
web2: 7+,8,9,10-
# Total number of registers used, followed by assignment to webs
registers: 2
r0: web0
r0: web1
r1: web2
```

When the assignment is infeasible the second section becomes
`registers: 0` followed by `M: webX` lines and a warning is printed on
stderr.

## Project layout

```
src/Graph.h               templated Graph<T>/Vertex<T>/Edge<T>
src/Web.{h,cpp}           live range / web / merge / interference test
src/Parser.{h,cpp}        parsers for the two input files
src/InterferenceBuilder.* webs → Graph<int>
src/Allocator.{h,cpp}     basic / spilling / splitting / free
src/OutputWriter.{h,cpp}  spec-formatted output
src/Menu.{h,cpp}          interactive CLI
src/main.cpp              entry point (batch / menu dispatch)
data/                     sample inputs (Figures 5, 7, 8 of the spec)
Doxyfile                  generates Documentation/html
```

## Documentation

```bash
doxygen Doxyfile     # writes ./Documentation/html
```

(or `cmake --build build --target doc` from the build directory if Doxygen
was located by CMake).

## Complexity summary

| stage | complexity |
| --- | --- |
| Parse ranges file        | O(F) on file size |
| Build webs (union-find)  | O(R² · L) — R ranges, L avg points each |
| Build interference graph | O(\|webs\|² · L) |
| Chaitin pass (one K)     | O(K · \|V\| · (\|V\| + \|E\|)) |
| Basic search of min K    | O(N · \|V\| · (\|V\| + \|E\|)) |
| Spilling / splitting     | × budget factor |
| DSatur (free)            | O(\|V\|² + \|V\| · \|E\|) |
