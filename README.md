# (SPOILERS) RAID-6 Data Recovery Challenge

## Project Overview

This project implements a RAID-6 data recovery system that recovers a video file from damaged RAID-6 arrays. The solution performs a **two-stage recovery process**:

1. Recover data from a 4-drive RAID-6 configuration.
2. Use the recovered data to solve an 8-drive RAID-6 recovery problem, ultimately producing a playable `.mp4` file.

---

## The Recovery Process

### Stage 1: 4-Drive Recovery

- Extract the initial 4 drives from `part_1.tar.gz`:
  ```bash
  tar -xvf part_1.tar.gz
  ```
  This creates the `drives/` directory containing: `sda`, `sdb`, `sdc`, and `sdd`.

- Analyze the drives:
  - Use `ls -lh` or `ls -la` to inspect file sizes.
  - You'll notice that `sdb` and `sdc` are significantly smaller than `sda` and `sdd`, indicating that they are corrupted.

- Perform RAID-6 recovery on these 4 drives.

- The recovery process produces `recovered.txt` containing:
  - A message
  - A base64-encoded string at the end.

- Extract the base64 portion and save it to `base64.txt`.

### 2 Stage 2: Extract Archive From Base64

- Convert the extracted base64 string to a `.tar.gz` file and extract it:
  ```bash
  base64 -d base64.txt | gunzip | tar -xvf -
  ```

- This produces sector files for 8 drives.

### Stage 3: 8-Drive R Recovery

- Input: 839 sectors of 512-byte blocks from 8 drives.
- Process:
  - Handles up to two simultaneous drive failures.
  - Uses RAID-6 algorithms with P and Q syndromes for reconstruction.
  - Processes sectors using precomputed Galois Field tables.
- Output: Base64-encoded video data (`base64.txt`).

### 4 Final Output

- Decode and extract the final video file from `base64.txt`, resulting in:

  ```
  video/ricky.mp4
  ```

---

## Directory Structure

```
.
├── build/               # Build directory for the executable
│   └── main             # Compiled recovery program
├── drives/              # Contains the four drives (sda, sdb, sdc, sdd)
├── src/                 # Source code files
│   ├── gf_raid6.c       # Galois Field arithmetic implementation
│   ├── gf_raid6.h       # Header for Galois Field functions
│   ├── main.c           # Main program flow
│   ├── raid6_recovery_1.c # 4-drive recovery implementation
│   └── raid6_recovery_2.c # 8-drive recovery implementation
├── Makefile             # Build instructions
├── part_1.tar.gz        # Archive containing the initial four drives
└── README.md            # This file
```

---

## Build Instructions

### Clone the repository

```bash
git clone https://github.com/TheActualRen/stderr-raid-recovery-challenge.git
cd stderr-raid-recovery-challenge
```

### Build the project

```bash
make
```

### Run the recovery

```bash
cd build
./main
```

---

## What the Program Does

- Extracts `part_1.tar.gz` to create the `drives/` directory.
- Performs the first RAID-6 recovery on the four drives.
- Extracts and decodes the base64 archive to get sector files.
- Performs the second RAID-6 recovery on the eight drives.
- Outputs the recovered video to `video/ricky.mp4`.

---

## Technical Details

- **Galois Field Arithmetic:**  
  Implements GF(256) multiplication using precomputed tables for efficiency.

- **RAID-6 Recovery:**  
  Supports both 4-drive and 8-drive configurations with up to two failed drives.

---

## Dependencies

- GCC compiler  
- Standard C library  
- POSIX-compliant system  
- `base64`, `gunzip`, and `tar` utilities
