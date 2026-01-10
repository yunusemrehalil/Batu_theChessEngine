# Batu Chess Engine

A UCI-compatible chess engine written in C++17 with neural network evaluation.

## Features

### Board Representation
- **Bitboard Representation**: 64-bit bitboards for efficient board state management
- **Magic Bitboards**: Pre-computed attack tables for sliding pieces (bishops, rooks, queens)

### Search
- **Alpha-Beta Search**: Negamax algorithm with alpha-beta pruning
- **Iterative Deepening**: Searches depth 1, 2, 3... with time management
- **Transposition Table**: Zobrist hashing with 2^20 entries (~32MB), stores EXACT/ALPHA/BETA bounds
- **Quiescence Search**: Resolves tactical positions by searching captures (max depth 8)
- **Move Ordering**: MVV-LVA + TT move priority + center control + development bonuses
- **Proper Mate Detection**: Returns `CHECKMATE_SCORE - ply` for shortest mate path

### Evaluation
- **Neural Network Evaluation**: 768→256→32→1 feedforward network
  - Input: 12 pieces × 64 squares = 768 binary features
  - Hidden layers: ReLU activation
  - Output: tanh scaled to centipawns (×600)
- **Fallback Evaluation**: Material counting when NN weights unavailable

### Interface
- **UCI Protocol**: Standard Universal Chess Interface for GUI compatibility
- **Time Control**: Supports `wtime`, `btime`, `movetime`, `depth`, `infinite`

## Architecture

```
Batu_theChessEngine/
├── main.cpp              # Entry point
├── CMakeLists.txt        # CMake build configuration
├── weights.txt           # Neural network weights (exported from training)
├── include/
│   ├── types.hpp         # Core types, constants, enums
│   ├── magic_numbers.hpp # Pre-computed magic bitboard tables
│   ├── position.hpp      # Position class (game state)
│   ├── attacks.hpp       # Attack table generation
│   ├── movegen.hpp       # Move generation and make_move
│   ├── search.hpp        # Alpha-beta search with TT integration
│   ├── nn_eval.hpp       # Neural network forward pass
│   ├── tt.hpp            # Transposition table with Zobrist hashing
│   └── uci.hpp           # UCI protocol + iterative deepening
├── training/
│   ├── train.py          # PyTorch training script
│   ├── positions.csv     # Training data (FEN + Stockfish eval)
│   └── checkpoints/      # Model checkpoints during training
└── README.md
```

## Neural Network

The engine uses a simple feedforward neural network trained on ~1M Stockfish-labeled positions.

### Architecture
```
Input (768) → Hidden1 (256, ReLU) → Hidden2 (32, ReLU) → Output (1, tanh)
```

### Input Encoding
- 12 piece types × 64 squares = 768 binary features
- Piece order: `P, R, N, B, Q, K, p, r, n, b, q, k` (matching `types.hpp` enum)
- Square indexing: a8=0, b8=1, ..., h1=63

### Training
```bash
cd training
python train.py
```
- Uses PyTorch with CUDA support (if available)
- MSE loss between predicted and Stockfish evaluations
- Exports weights to `weights.txt` for C++ engine

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
