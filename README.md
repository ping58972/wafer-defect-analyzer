# Wafer Defect Analyzer

A C++20 command-line tool and reusable library for analyzing synthetic wafer maps. Given a grid of die-test results, it calculates yield and groups edge-adjacent defects into clusters with sizes and bounding boxes.

## Build and run

Requires CMake 3.20+ and a C++20 compiler.

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
ctest --test-dir build -C Release --output-on-failure
./build/wafer-inspect examples/sample.csv
```

With a Windows Visual Studio generator, run `build/Release/wafer-inspect.exe examples/sample.csv`.

```text
rows=5 cols=5 passed=16 defective=5 outside=4 clusters=3
yield=76.19%
cluster,size,top,left,bottom,right
1,1,0,3,0,3
2,3,1,1,2,2
3,1,3,3,3,3
```

The input is a headerless CSV grid with one symbol per cell. Use `P` for a passing die, `D` for a defective die, and `.` for an outside-wafer site. Grids must be rectangular and between 1×1 and 1024×1024. Coordinates are zero-based with the origin at the upper left. An all-outside map reports `yield=N/A`. Invalid input prints an error to standard error and returns a nonzero exit status.

## Algorithm

Each defective cell is a graph vertex. Two vertices share an edge when their cells touch horizontally or vertically. Starting from each unvisited defect in row-major order, breadth-first search marks every reachable defect and extends the cluster's bounding box. Diagonally touching defects remain in separate clusters unless connected by a path of edge-adjacent defects.

For a map with `R` rows and `C` columns, the scan visits `R × C` cells and checks at most four neighbors per defect. Time and worst-case memory are both `O(R × C)`. If `P` and `D` are the passing and defective counts, yield is `100 × P / (P + D)` percent. The sample inspects `16 + 5 = 21` dies, so its yield is `100 × 16 / 21 = 76.19%`.

## Skills and source guide

| Source | Techniques |
| --- | --- |
| [Library interface](include/wafer_map.hpp) | C++20 value types, encapsulation, validated map representation, and reusable analysis API. |
| [Map implementation](src/wafer_map.cpp) | Standard-library containers, bounded stream parsing, exception handling, BFS, deterministic traversal, and yield calculation. |
| [Command-line tool](src/main.cpp) | RAII file I/O, machine-readable cluster rows, human-readable summaries, and process exit codes. |
| [Build and tests](CMakeLists.txt) | CMake library and executable targets, warnings as errors, and CTest integration. |

The six library test cases cover counts, yield, connectivity, row boundaries, all-outside maps, malformed input, and maximum dimensions. CTest also invokes the CLI on the checked-in [synthetic sample](examples/sample.csv). The input is a simplified inspection fixture whose cells already contain pass/fail labels.

All seven CTest cases passed on Apple Silicon with Apple Clang, both in a Release build and with AddressSanitizer and UndefinedBehaviorSanitizer enabled.
