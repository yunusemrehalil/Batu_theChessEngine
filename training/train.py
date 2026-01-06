"""
Chess Neural Network Training Script
Architecture: 768 -> 256 -> 32 -> 1 (matching nn_eval.hpp)
"""

import torch
import torch.nn as nn
from torch.utils.data import Dataset, DataLoader
import os
import time
import random

# =============================================================================
# Configuration
# =============================================================================

CONFIG = {
    "data_file": "positions.csv",
    "output_dir": "checkpoints",
    "weights_file": "../weights.txt",
    
    "max_samples": 1_000_000,  # Use random 1M from 13M dataset (None = use all)
    "epochs": 10,
    "batch_size": 4096,
    "learning_rate": 0.001,
    "val_split": 0.1,
    
    "scale_factor": 600.0,
    "mate_score": 10000,
    "clamp_score": 4000,
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


def load_dataset(filepath: str):
    """
    Load and parse CSV dataset.
    Returns list of (fen, normalized_target) tuples.
    """
    data = []
    skipped = 0
    
    print(f"Loading dataset: {filepath}")
    start_time = time.time()
    
    with open(filepath, 'r') as f:
        # Skip header
        header = next(f)
        print(f"Header: {header.strip()}")
        
        for line_num, line in enumerate(f, start=1):
            if line_num % 1_000_000 == 0:
                elapsed = time.time() - start_time
                print(f"  Processed {line_num:,} lines ({elapsed:.1f}s)")
            
            line = line.strip()
            if not line:
                continue
            
            # Parse line (use rsplit to handle FENs with commas in move counters)
            parts = line.rsplit(',', 1)
            if len(parts) != 2:
                skipped += 1
                continue
            
            fen, eval_str = parts
            
            try:
                # Parse and clamp score
                score = parse_evaluation(eval_str)
                score = max(-CONFIG["clamp_score"], min(CONFIG["clamp_score"], score))
                
                # Normalize to [-1, 1] using tanh
                normalized = float(torch.tanh(torch.tensor(score / CONFIG["scale_factor"])))
                
                data.append((fen, normalized))
                
            except (ValueError, KeyError) as e:
                skipped += 1
                continue
    
    elapsed = time.time() - start_time
    print(f"Loaded {len(data):,} positions in {elapsed:.1f}s (skipped {skipped:,})")
    
    # Random sampling if max_samples is set
    if CONFIG["max_samples"] and len(data) > CONFIG["max_samples"]:
        print(f"Sampling {CONFIG['max_samples']:,} random positions from {len(data):,}...")
        data = random.sample(data, CONFIG["max_samples"])
    
    return data

# =============================================================================
# Neural Network (matches nn_eval.hpp exactly)
# =============================================================================

class ChessNet(nn.Module):
    """
    Simple feedforward network for position evaluation.
    Architecture: 768 -> 256 (ReLU) -> 32 (ReLU) -> 1 (tanh)
    """
    
    def __init__(self):
        super().__init__()
        self.fc1 = nn.Linear(768, 256)
        self.fc2 = nn.Linear(256, 32)
        self.fc3 = nn.Linear(32, 1)
    
    def forward(self, x):
        x = torch.relu(self.fc1(x))
        x = torch.relu(self.fc2(x))
        x = torch.tanh(self.fc3(x))
        return x

# =============================================================================
# Weight Export (matches nn_eval.hpp load_weights format)
# =============================================================================

def export_weights(model: nn.Module, filepath: str):
    """
    Export weights to plain text format for C++ engine.
    
    Order (matching nn_eval.hpp):
        1. fc1.weight (transposed to [768, 256] for row-major)
        2. fc1.bias [256]
        3. fc2.weight (transposed to [256, 32])
        4. fc2.bias [32]
        5. fc3.weight (transposed to [32, 1])
        6. fc3.bias [1]
    
    C++ loads: weights_input_hidden1[i * HIDDEN1_SIZE + j]
    PyTorch stores: weight[out_features, in_features]
    So we transpose: weight.T[in_features, out_features]
    """
    model.eval()
    
    with open(filepath, 'w') as f:
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
    
    total_params = 768*256 + 256 + 256*32 + 32 + 32*1 + 1
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
    criterion = nn.MSELoss()
    
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
        
        # Log results
        print(f"Epoch {epoch}/{CONFIG['epochs']} | "
              f"Train Loss: {train_loss:.6f} | "
              f"Val Loss: {val_loss:.6f} | "
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
