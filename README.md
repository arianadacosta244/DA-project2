# Compiler Register Allocation Algorithms

FEUP LEIC016 / DA — *Spring 2026, Programming Project II*

A C++17 tool that reads variable live-range descriptions and a register
budget, builds the interference graph, and tries to color it. Four algorithm
variants are supported:

| algorithm | description |
| --- | --- |
| `basic` | Chaitin simplify-and-pop, no spilling. |
| `spilling, K` | Same, but allowed to spill up to K webs to memory. |
| `splitting, K` | Same, but allowed to split up to K webs at their definition points. |
| `free` | DSatur (Brélaz, 1979) coloring. |

## Build

```bash
make
# or
cmake -S . -B build && cmake --build build
```

Produces `build/regalloc`.

## Use

### Interactive menu
```bash
./build/regalloc
```

### Batch mode
```bash
./build/regalloc -b ranges.txt registers.txt allocation.txt
```

Exit codes: `0` feasible, `1` infeasible, `2` parse / I/O error.

## Project layout

```
src/
  main.cpp
  core/   Graph.h, Web.h, Web.cpp
  io/     Parser.h, Parser.cpp, OutputWriter.h, OutputWriter.cpp
  algo/   InterferenceBuilder.h, InterferenceBuilder.cpp, Allocator.h, Allocator.cpp
  ui/     Menu.h, Menu.cpp
data/     sample inputs
```
