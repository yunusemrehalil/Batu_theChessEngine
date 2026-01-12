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

### Search Optimizations
- **Null Move Pruning (NMP)**: Skips search for positions where opponent can't beat beta
  - Depth ≥ 3, not in check, eval ≥ beta, non-pawn material present
  - Reduction R=3: searches at `depth - 1 - R` (effectively depth-4)
- **Late Move Reductions (LMR)**: Reduces depth for late quiet moves
  - Depth ≥ 3, move count ≥ 4, quiet moves only, not in check
  - Log-based reduction: `R = 0.75 + log(depth) * log(move_count) / 2.25`
  - Killers reduced less (proven good at this ply)
- **Delta Pruning**: Prunes futile captures in quiescence search
  - Skips captures where `stand_pat + captured_value + 200 < alpha`
- **Killer Moves**: Stores 2 quiet moves per ply that caused beta cutoffs
  - Searched with high priority after captures

### Move Ordering
- **Priority (highest to lowest)**:
  1. TT move (hash move from transposition table)
  2. Captures (MVV-LVA: Most Valuable Victim - Least Valuable Attacker)
  3. Killer moves (quiet moves that caused beta cutoffs)
  4. Quiet moves (promotions, center control, development bonuses)
- **Proper Mate Detection**: Returns `CHECKMATE_SCORE - ply` for shortest mate path

### Evaluation
- **Neural Network Evaluation**: 768→256→32→1 feedforward network
  - Input: 12 pieces × 64 squares = 768 binary features
  - Hidden layers: ReLU activation
  - Output: tanh scaled to centipawns (×600)
  - **Sparse First-Layer Optimization**: Only computes non-zero inputs (~32 active pieces max)
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

The engine uses a feedforward neural network with PSQT skip connection, trained on ~1M labeled positions.

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

**Date**: January 11, 2026  
**System**: Windows with MSVC, Release optimization  
**Configuration**: Alpha-Beta + TT + NMP + LMR + Killers + NN Eval

```
Starting Position (depth 7): 2460 ms, 349826 nodes, score 29, best g2g3
Eval Test (SF: -6.0) (depth 6): 51 ms, 11098 nodes, score -199, best c8d7
Mate in 3 (depth 10): 2832 ms, 877343 nodes, score 11107, best a8a4
Mate in 2 (depth 6): 4754 ms, 753786 nodes, score 11109, best h8e5

Total: ~10s, ~2M nodes, ~200k nps
```

### Test Positions
| Position | FEN | Expected |
|----------|-----|----------|
| Starting | `rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1` | ~0cp |
| Eval Test | `K1B5/P2r4/1p1r1n2/4k3/8/3PPP2/8/8 w - - 0 1` | SF: -600cp |
| Mate in 3 | `Q7/8/2K5/8/4N2R/3P4/3Pk3/8 w - - 0 1` | Find mate |
| Mate in 2 | `7B/3B1p2/rP1p2R1/n2k1Pb1/N2Pp3/4P3/K2nN1r1/2R5 w - - 0 1` | Find mate |

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
