# Batu Chess Engine

A UCI-compatible chess engine written in C++17.

## Features

- **Bitboard Representation**: 64-bit bitboards for efficient board state management
- **Magic Bitboards**: Pre-computed attack tables for sliding pieces (bishops, rooks, queens)
- **Alpha-Beta Search**: Negamax algorithm with alpha-beta pruning
- **Move Ordering**: MVV-LVA (Most Valuable Victim - Least Valuable Attacker) + center control + development bonuses
- **UCI Protocol**: Standard Universal Chess Interface for GUI compatibility
- **Static Evaluation**: Material counting with piece-square considerations (prepared for neural network replacement)

## Architecture

```
Batu_theChessEngine/
├── main.cpp              # Entry point
├── CMakeLists.txt        # CMake build configuration
├── include/
│   ├── types.hpp         # Core types, constants, enums
│   ├── magic_numbers.hpp # Pre-computed magic bitboard tables
│   ├── position.hpp      # Position class (game state)
│   ├── attacks.hpp       # Attack table generation
│   ├── movegen.hpp       # Move generation and make_move
│   ├── search.hpp        # Alpha-beta search with move ordering
│   └── uci.hpp           # UCI protocol implementation
└── README.md
```

## Building

### Using g++ (MinGW/MSYS2)
```bash
g++ -std=c++17 -O3 -o batu.exe main.cpp
```

### Using CMake
```bash
mkdir build && cd build
cmake ..
cmake --build . --config Release
```

## Usage

Run the engine and use UCI commands:
```
./batu.exe
uci
position startpos
go depth 6
quit
```

### Benchmark
```
./batu.exe
bench
quit
```

## Benchmark Results

**Date**: January 6, 2026  
**System**: Windows with MinGW g++ 13.2.0, -O3 optimization  
**Configuration**: Alpha-Beta Pruning + Move Ordering (MVV-LVA)

### Starting Position
`rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1`

| Depth | Time (ms) | Nodes | Best Move |
|-------|-----------|-------|-----------|
| 5 | 25 | 150,834 | e2e4 |
| 6 | 98 | 1,096,768 | b1c3 |
| 7 | 859 | 6,428,787 | b1c3 |

### Position 2 (Kiwipete)
`r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq -`

| Depth | Time (ms) | Nodes | Best Move |
|-------|-----------|-------|-----------|
| 5 | 442 | 795,043 | e2a6 |
| 6 | 1,453 | 9,036,672 | e2a6 |
| 7 | 10,756 | 31,160,887 | d5e6 |

### Performance Improvement (vs. Legacy Code from Nov 2023)

| Position | Depth | Old Time | New Time | Speedup |
|----------|-------|----------|----------|---------|
| Starting | 6 | 991 ms | 98 ms | **~10x faster** |
| Starting | 7 | 17,665 ms | 859 ms | **~20x faster** |
| Position 2 | 5 | 3,079 ms | 442 ms | **~7x faster** |

---

## Historical Benchmark Data

<details>
<summary>Legacy benchmarks (October-November 2023)</summary>

### 27/10/2023 - Move Generation Speed Tests

| Configuration | Iterations | Time |
|--------------|------------|------|
| With sort | 1,000 | 80.40 seconds |
| Without sort | 1,000 | 0.007 seconds |
| Without sort | 1,000,000 | 6.37 seconds |
| Without sort | 10,000,000 | 73.85 seconds |

### 02/11/2023 - Without Alpha-Beta Pruning

**Starting Position (Depth 6)**
- Without sort: 9,548 ms (119,060,324 nodes)
- With sort: 15,356 ms (119,060,324 nodes)

### 05/11/2023 - With Alpha-Beta Pruning

**Starting Position**
- Depth 6: 991 ms
- Depth 7: 17,665 ms

**Position 2**
- Depth 5: 3,079 ms
- Depth 6: 50,249 ms

</details>

---

## Author

**Yunus Emre Halil**

## License

This project is open source.  
