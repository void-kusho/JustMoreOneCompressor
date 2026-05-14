# JustMoreOneCompressor - Huffman File Compressor

## Overview

A command-line file compressor built in C++ using Huffman coding algorithm. This project demonstrates fundamental data structures and algorithms including priority queues, binary trees, and bit-level I/O — all implemented without heap allocation (no `new`, `malloc`, or dynamic memory).

## Features

- **Lossless compression** using Huffman coding
- **Binary-safe** — works with text and binary files
- **Zero heap allocation** — all memory is stack/static allocated
- **Single-file implementation** — everything in `main.cpp`
- **Round-trip verified** — decompressed output matches original byte-for-byte

## Building

Requires a C++17 compatible compiler (g++, clang++):

```bash
g++ -std=c++17 -Wall -Wextra -O2 -o huffman main.cpp
```

## Usage

```bash
# Compress a file
./huffman compress <input_file> <output_file>

# Decompress a file
./huffman decompress <input_file> <output_file>
```

### Examples

```bash
# Compress a text file
./huffman compress document.txt document.huff

# Decompress it back
./huffman decompress document.huff document_restored.txt

# Verify round-trip
diff document.txt document_restored.txt
```

## Compressed File Format

| Offset      | Size     | Description                          |
|-------------|----------|--------------------------------------|
| 0           | 8 bytes  | Original file size (little-endian)   |
| 8           | 4 bytes  | Tree header length (little-endian)   |
| 12          | variable | Serialized Huffman tree (pre-order)  |
| 12+tree_len | variable | Compressed bitstream                 |
| EOF-8       | 8 bytes  | Total bit count (metadata)           |

### Tree Serialization

The Huffman tree is stored as a pre-order traversal:
- `0x00` — internal node (followed by left subtree, then right subtree)
- `0x01` + 1 byte symbol — leaf node

## Algorithm

1. **Frequency counting** — two-pass: first pass counts byte frequencies
2. **Tree construction** — repeatedly merge two lowest-frequency nodes
3. **Code generation** — depth-first traversal assigns bit codes
4. **Compression** — replace each byte with its Huffman code
5. **Decompression** — walk the tree bit-by-bit to recover symbols

## Design Decisions

- **No heap allocation**: All arrays are fixed-size static/storage duration. Maximum supported alphabet is 256 (all byte values), with up to 512 tree nodes.
- **I/O buffering**: 4096-byte read/write buffers for performance.
- **Bit-level I/O**: Custom bit reader/writer structs handle packing bits into bytes.

## Performance

| Input Type        | Size     | Compressed | Ratio   |
|-------------------|----------|------------|---------|
| Repetitive text   | 45,000 B | 25,375 B   | 56.4%   |
| Random binary     | 10,240 B | 10,238 B   | 99.98%  |

Note: Random data cannot be compressed (expected). Text with repeated patterns compresses well.

## Development Environment

- [Helix Editor](https://helix-editor.com/)
- [clangd](https://clangd.llvm.org/)
- [GCC](https://gcc.gnu.org/) (g++ with C++17)

## Project Structure

```
JustMoreOneCompressor/
├── main.cpp          # Full Huffman compressor (single file)
├── README.md         # This file
├── TIMELOG.md        # Time tracking log
├── LEARNING.md       # Discussion of learning strategies
├── LICENSE           # Project license
└── .gitignore        # Build artifacts ignored
```

## About C++

C++ is a high-performance, general-purpose programming language known for its efficiency and flexibility. Created by Bjarne Stroustrup as an extension of C, it supports object-oriented, procedural, and generic programming paradigms, making it ideal for system software, game engines, and resource-intensive applications.

## Useful Websites

- [W3Schools C++ Reference](https://www.w3schools.com/cpp/cpp_ref_reference.asp)
- [C++ Tutorial](https://cplusplus.com/doc/tutorial/)
- [Huffman Coding - Wikipedia](https://en.wikipedia.org/wiki/Huffman_coding)
- [cppreference.com](https://en.cppreference.com/)
