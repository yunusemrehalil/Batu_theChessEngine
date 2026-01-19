"""
Chess Neural Network Training Script

Architecture: 768 -> 256 -> 32 -> 1 with PSQT skip connection

Key architectural insight:
- PSQT layer provides direct linear path for material values
- Deeper layers learn positional adjustments only
- Initialization with standard piece values gives huge head start
"""

import torch
import torch.nn as nn
from torch.utils.data import Dataset, DataLoader
import os
import time
import random

# =============================================================================
# Reproducibility - Set seeds for consistent training runs
# =============================================================================

SEED = 42
random.seed(SEED)
torch.manual_seed(SEED)
if torch.cuda.is_available():
    torch.cuda.manual_seed(SEED)
    torch.cuda.manual_seed_all(SEED)  # For multi-GPU
# =============================================================================
# Configuration
# =============================================================================

CONFIG = {
    "data_file": "positions.csv",
    "output_dir": "checkpoints",
    "weights_file": "../weights.txt",
    
    "max_samples": 2_000_000,  # Total samples to use
    "epochs": 10,
    "batch_size": 4096,
    "learning_rate": 0.001,
    "val_split": 0.1,
    
    "scale_factor": 600.0,
    "mate_score": 10000,
    "clamp_score": 4000,
    
    # Stratified sampling: ensure good distribution across eval ranges
    "use_stratified_sampling": True,
    "eval_buckets": [
        (-10000, -500, 0.15),   # Large Black advantage: 15%
        (-500, -200, 0.15),    # Medium Black advantage: 15%
        (-200, -50, 0.10),     # Small Black advantage: 10%
        (-50, 50, 0.20),       # Equal positions: 20%
        (50, 200, 0.10),       # Small White advantage: 10%
        (200, 500, 0.15),      # Medium White advantage: 15%
        (500, 10000, 0.15),    # Large White advantage: 15%
    ],
}

# =============================================================================
# Piece Mapping (must match types.hpp enum order)
# enum Piece { P, R, N, B, Q, K, p, r, n, b, q, k, NO_PIECE };
# =============================================================================

PIECE_TO_INDEX = {
    'P': 0,  'R': 1,  'N': 2,  'B': 3,  'Q': 4,  'K': 5,   # White
    'p': 6,  'r': 7,  'n': 8,  'b': 9,  'q': 10, 'k': 11,  # Black
}

# =============================================================================
# FEN Parsing
# =============================================================================

def fen_to_tensor(fen: str) -> torch.Tensor:
    """
    Convert FEN position to 768-dimensional input tensor.
    Layout: 12 pieces x 64 squares (piece-major ordering)
    Square indexing: a8=0, b8=1, ..., h1=63 (matches FEN traversal)
    """
    tensor = torch.zeros(768, dtype=torch.float32)
    
    board_part = fen.split()[0]
    square = 0
    
    for char in board_part:
        if char == '/':
            continue
        elif char.isdigit():
            square += int(char)
        elif char in PIECE_TO_INDEX:
            piece_idx = PIECE_TO_INDEX[char]
            tensor[piece_idx * 64 + square] = 1.0
            square += 1
    
    return tensor


def parse_evaluation(eval_str: str) -> int:
    """
    Parse Stockfish evaluation string to centipawns.
    Formats: "+56", "-10", "#5" (mate in 5), "#-3" (mated in 3)
    """
    eval_str = eval_str.strip()
    
    if '#' in eval_str:
        # Checkmate notation
        mate_str = eval_str.replace('#', '')
        mate_in = int(mate_str)
        return CONFIG["mate_score"] if mate_in > 0 else -CONFIG["mate_score"]
    else:
        # Regular centipawn score
        return int(eval_str.replace('+', ''))

# =============================================================================
# Dataset
# =============================================================================

class ChessDataset(Dataset):
    """
    Lazy-loading dataset for chess positions.
    Stores FEN strings in memory, converts to tensors on access.
    Handles 16M+ positions without OOM.
    """
    
    def __init__(self, data: list):
        """
        Args:
            data: List of (fen, target) tuples where target is normalized [-1, 1]
        """
        self.data = data
    
    def __len__(self):
        return len(self.data)
    
    def __getitem__(self, idx):
        fen, target = self.data[idx]
        return fen_to_tensor(fen), torch.tensor(target, dtype=torch.float32)


def reservoir_sample(reservoir: list, item, max_size: int, count: int):
    """
    Reservoir sampling: maintains a random sample of max_size items
    from a stream of 'count' items seen so far.
    
    Args:
        reservoir: Current list of sampled items (modified in place)
        item: New item to potentially add
        max_size: Maximum reservoir size
        count: Total items seen so far (1-indexed, including current item)
    """
    if len(reservoir) < max_size:
        reservoir.append(item)
    else:
        # Replace existing item with probability max_size/count
        j = random.randint(0, count - 1)
        if j < max_size:
            reservoir[j] = item


def load_dataset(filepath: str):
    """
    Load and parse CSV dataset with stratified reservoir sampling.
    Uses reservoir sampling per bucket to keep memory bounded regardless
    of input file size (handles 13M+ rows without OOM).
    
    Returns list of (fen, normalized_target) tuples.
    """
    # Calculate target size per bucket for reservoir sampling
    bucket_targets = {}
    if CONFIG["use_stratified_sampling"] and CONFIG["max_samples"]:
        for bucket_idx, (_, _, ratio) in enumerate(CONFIG["eval_buckets"]):
            bucket_targets[bucket_idx] = int(CONFIG["max_samples"] * ratio)
    else:
        # Non-stratified: all goes to bucket 0
        bucket_targets[0] = CONFIG["max_samples"] if CONFIG["max_samples"] else float('inf')
    
    # Reservoirs (bounded memory) and counters (for reservoir sampling probability)
    reservoirs = {i: [] for i in bucket_targets.keys()}
    bucket_counts = {i: 0 for i in bucket_targets.keys()}  # Total seen per bucket
    skipped = 0
    total_loaded = 0
    
    print(f"Loading dataset: {filepath}")
    print(f"Using reservoir sampling (memory-bounded) for {len(bucket_targets)} buckets")
    start_time = time.time()
    
    with open(filepath, 'r') as f:
        header = next(f)
        print(f"Header: {header.strip()}")
        
        for line_num, line in enumerate(f, start=1):
            if line_num % 1_000_000 == 0:
                elapsed = time.time() - start_time
                print(f"  Processed {line_num:,} lines ({elapsed:.1f}s)")
            
            line = line.strip()
            if not line:
                continue
            
            parts = line.rsplit(',', 1)
            if len(parts) != 2:
                skipped += 1
                continue
            
            fen, eval_str = parts
            
            try:
                score = parse_evaluation(eval_str)
                clamped = max(-CONFIG["clamp_score"], min(CONFIG["clamp_score"], score))
                normalized = float(torch.tanh(torch.tensor(clamped / CONFIG["scale_factor"])))
                
                item = (fen, normalized)
                
                # Find bucket for this position
                if CONFIG["use_stratified_sampling"]:
                    for bucket_idx, (low, high, _) in enumerate(CONFIG["eval_buckets"]):
                        if low <= score < high:
                            bucket_counts[bucket_idx] += 1
                            reservoir_sample(
                                reservoirs[bucket_idx], 
                                item, 
                                bucket_targets[bucket_idx],
                                bucket_counts[bucket_idx]
                            )
                            break
                else:
                    bucket_counts[0] += 1
                    reservoir_sample(
                        reservoirs[0], 
                        item, 
                        bucket_targets[0],
                        bucket_counts[0]
                    )
                
                total_loaded += 1
                
            except (ValueError, KeyError):
                skipped += 1
                continue
    
    elapsed = time.time() - start_time
    print(f"Loaded {total_loaded:,} positions in {elapsed:.1f}s (skipped {skipped:,})")
    
    # Combine reservoirs into final dataset
    data = []
    if CONFIG["use_stratified_sampling"]:
        print("\nStratified reservoir sampling results:")
        for bucket_idx, (low, high, ratio) in enumerate(CONFIG["eval_buckets"]):
            sampled = len(reservoirs[bucket_idx])
            seen = bucket_counts[bucket_idx]
            target = bucket_targets[bucket_idx]
            data.extend(reservoirs[bucket_idx])
            print(f"  Bucket [{low:+6d}, {high:+6d}): {sampled:,} sampled from {seen:,} seen (target: {target:,})")
    else:
        data.extend(reservoirs[0])
    
    random.shuffle(data)
    print(f"\nTotal sampled: {len(data):,}")
    
    return data

# =============================================================================
# Neural Network with PSQT Skip Connection
# =============================================================================

class ChessNet(nn.Module):
    """
    Neural network with PSQT (Piece-Square Table) skip connection.
    
    Architecture:
        Input (768) ──┬──> PSQT (768->1, linear) ────────────┐
                      │                                       │
                      └──> fc1 (768->256, ReLU)              │
                              │                               │
                           fc2 (256->32, ReLU)               │
                              │                               │
                           fc3 (32->1) ──> positional        │
                                               │              │
                                               └──> + <───────┘
                                                    │
                                                 tanh(output)
    
    Key insight:
    - Material values flow directly through linear PSQT layer
    - Deeper layers learn positional adjustments only
    - No need to learn basic piece values through nonlinear layers
    """
    
    def __init__(self):
        super().__init__()
        # Positional evaluation path (learns positional patterns)
        self.fc1 = nn.Linear(768, 256)
        self.fc2 = nn.Linear(256, 32)
        self.fc3 = nn.Linear(32, 1)
        
        # PSQT skip connection - direct material path (no bias needed)
        self.psqt = nn.Linear(768, 1, bias=False)
        self._init_psqt()
    
    def _init_psqt(self):
        """
        Initialize PSQT weights with standard piece values.
        This gives the network a huge head start - it already knows
        basic material values and just needs to learn refinements.
        
        Piece order (from types.hpp): P, R, N, B, Q, K, p, r, n, b, q, k
        Values scaled by 1/600 to match tanh output range.
        """
        # Standard piece values in centipawns
        piece_values = [
            100,   # P (white pawn)
            500,   # R (white rook)  
            320,   # N (white knight)
            330,   # B (white bishop)
            900,   # Q (white queen)
            0,     # K (white king - infinite, use 0)
            -100,  # p (black pawn)
            -500,  # r (black rook)
            -320,  # n (black knight)
            -330,  # b (black bishop)
            -900,  # q (black queen)
            0,     # k (black king)
        ]
        
        with torch.no_grad():
            for piece_idx, value in enumerate(piece_values):
                for square in range(64):
                    # Scale to tanh range: value/600
                    self.psqt.weight[0, piece_idx * 64 + square] = value / 600.0
    
    def forward(self, x):
        # Material contribution (linear skip connection - trivial to learn)
        material = self.psqt(x)
        
        # Positional contribution (learned through nonlinear layers)
        h = torch.relu(self.fc1(x))
        h = torch.relu(self.fc2(h))
        positional = self.fc3(h)
        
        # Combine and apply tanh for bounded output
        return torch.tanh(material + positional)

# =============================================================================
# Weight Export (matches nn_eval.hpp load_weights format)
# =============================================================================

def export_weights(model: nn.Module, filepath: str):
    """
    Export weights to plain text format for C++ engine.
    
    Order (matching nn_eval.hpp with PSQT):
        1. psqt.weight [768] - PSQT skip connection weights
        2. fc1.weight (transposed to [768, 256] for row-major)
        3. fc1.bias [256]
        4. fc2.weight (transposed to [256, 32])
        5. fc2.bias [32]
        6. fc3.weight (transposed to [32, 1])
        7. fc3.bias [1]
    
    C++ loads: weights_input_hidden1[i * HIDDEN1_SIZE + j]
    PyTorch stores: weight[out_features, in_features]
    So we transpose: weight.T[in_features, out_features]
    """
    model.eval()
    
    with open(filepath, 'w') as f:
        # PSQT weights first [768] - the skip connection for material
        for val in model.psqt.weight.detach().cpu().numpy().flatten():
            f.write(f"{val:.8f}\n")
        
        # fc1: input -> hidden1
        weight = model.fc1.weight.detach().cpu().numpy().T  # [768, 256]
        for val in weight.flatten():
            f.write(f"{val:.8f}\n")
        for val in model.fc1.bias.detach().cpu().numpy():
            f.write(f"{val:.8f}\n")
        
        # fc2: hidden1 -> hidden2
        weight = model.fc2.weight.detach().cpu().numpy().T  # [256, 32]
        for val in weight.flatten():
            f.write(f"{val:.8f}\n")
        for val in model.fc2.bias.detach().cpu().numpy():
            f.write(f"{val:.8f}\n")
        
        # fc3: hidden2 -> output
        weight = model.fc3.weight.detach().cpu().numpy().T  # [32, 1]
        for val in weight.flatten():
            f.write(f"{val:.8f}\n")
        for val in model.fc3.bias.detach().cpu().numpy():
            f.write(f"{val:.8f}\n")
    
    total_params = 768 + 768*256 + 256 + 256*32 + 32 + 32*1 + 1
    print(f"Exported {total_params:,} parameters to {filepath}")

# =============================================================================
# Training
# =============================================================================

def train():
    # Device
    device = torch.device("cuda" if torch.cuda.is_available() else "cpu")
    print(f"Device: {device}")
    if device.type == "cuda":
        print(f"GPU: {torch.cuda.get_device_name(0)}")
    
    # Load data
    all_data = load_dataset(CONFIG["data_file"])
    
    # Shuffle unconditionally before split (stratified path already shuffles,
    # but this ensures non-stratified path doesn't split in file order)
    random.shuffle(all_data)
    
    # Train/validation split
    val_size = int(len(all_data) * CONFIG["val_split"])
    train_size = len(all_data) - val_size
    
    train_data = all_data[:train_size]
    val_data = all_data[train_size:]
    
    print(f"Train: {len(train_data):,} | Validation: {len(val_data):,}")
    
    # DataLoaders
    train_loader = DataLoader(
        ChessDataset(train_data),
        batch_size=CONFIG["batch_size"],
        shuffle=True,
        num_workers=0,
        pin_memory=(device.type == "cuda")
    )
    
    val_loader = DataLoader(
        ChessDataset(val_data),
        batch_size=CONFIG["batch_size"],
        shuffle=False,
        num_workers=0,
        pin_memory=(device.type == "cuda")
    )
    
    # Model
    model = ChessNet().to(device)
    optimizer = torch.optim.Adam(model.parameters(), lr=CONFIG["learning_rate"])
    scheduler = torch.optim.lr_scheduler.ReduceLROnPlateau(
        optimizer, mode='min', factor=0.5, patience=3, verbose=True
    )
    # SmoothL1Loss (Huber-like) is more robust to outliers than MSE
    # Chess evals can have noise/extreme values, and tanh saturates
    criterion = nn.SmoothL1Loss()
    
    # Create checkpoint directory
    os.makedirs(CONFIG["output_dir"], exist_ok=True)
    
    # Training loop
    best_val_loss = float('inf')
    
    print(f"\nStarting training for {CONFIG['epochs']} epochs...")
    print("-" * 60)
    
    for epoch in range(1, CONFIG["epochs"] + 1):
        epoch_start = time.time()
        
        # Training phase
        model.train()
        train_loss = 0.0
        
        for batch_idx, (inputs, targets) in enumerate(train_loader):
            inputs = inputs.to(device)
            targets = targets.to(device).unsqueeze(1)
            
            optimizer.zero_grad()
            outputs = model(inputs)
            loss = criterion(outputs, targets)
            loss.backward()
            optimizer.step()
            
            train_loss += loss.item()
            
            # Progress update every 500 batches
            if (batch_idx + 1) % 500 == 0:
                print(f"  Epoch {epoch} | Batch {batch_idx + 1}/{len(train_loader)} | Loss: {loss.item():.6f}")
        
        train_loss /= len(train_loader)
        
        # Validation phase
        model.eval()
        val_loss = 0.0
        
        with torch.no_grad():
            for inputs, targets in val_loader:
                inputs = inputs.to(device)
                targets = targets.to(device).unsqueeze(1)
                outputs = model(inputs)
                val_loss += criterion(outputs, targets).item()
        
        val_loss /= len(val_loader)
        epoch_time = time.time() - epoch_start
        
        # Step learning rate scheduler
        scheduler.step(val_loss)
        
        # Log results
        current_lr = optimizer.param_groups[0]['lr']
        print(f"Epoch {epoch}/{CONFIG['epochs']} | "
              f"Train Loss: {train_loss:.6f} | "
              f"Val Loss: {val_loss:.6f} | "
              f"LR: {current_lr:.6f} | "
              f"Time: {epoch_time:.1f}s")
        
        # Save best model
        if val_loss < best_val_loss:
            best_val_loss = val_loss
            torch.save(model.state_dict(), os.path.join(CONFIG["output_dir"], "best_model.pt"))
            print(f"  -> New best model saved!")
        
        # Periodic checkpoint
        if epoch % 5 == 0:
            torch.save({
                'epoch': epoch,
                'model_state_dict': model.state_dict(),
                'optimizer_state_dict': optimizer.state_dict(),
                'train_loss': train_loss,
                'val_loss': val_loss,
            }, os.path.join(CONFIG["output_dir"], f"checkpoint_epoch_{epoch}.pt"))
    
    print("-" * 60)
    print(f"Training complete! Best validation loss: {best_val_loss:.6f}")
    
    # Load best model and export weights
    model.load_state_dict(torch.load(os.path.join(CONFIG["output_dir"], "best_model.pt")))
    export_weights(model, CONFIG["weights_file"])
    
    # Also save final PyTorch model
    torch.save(model.state_dict(), os.path.join(CONFIG["output_dir"], "final_model.pt"))
    print("Done!")


if __name__ == "__main__":
    train()
