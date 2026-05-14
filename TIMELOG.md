# Time Log — Huffman File Compressor

**Student:** Gabriel Chagas Boff
**Module:** Language – C++ (Module #1)
**Project:** File compression and decompression CLI tool

## Schedule

### First Week of Sprint

| Day       | Activity                                      | Hours |
|-----------|-----------------------------------------------|-------|
| Monday    | Compression Algorithms — researched Huffman coding, studied prefix codes, compared with other algorithms (LZ77, RLE) | 3.0   |
| Tuesday   | Software Planning — designed CLI interface, defined file format, outlined data structures | 2.5   |
| Wednesday | Software Planning — refined architecture, decided on no-heap constraint, planned tree representation with arrays | 2.5   |
| Thursday  | Setup CLI — created project repo, wrote argument parsing, set up build system with g++ | 2.0   |
| Friday    | Core Implementation — built frequency counting, Huffman tree construction, and code generation | 3.0   |
| Saturday  | Core Tests — tested frequency table against known inputs, verified tree structure, validated generated codes | 2.0   |
| **Subtotal** |                                             | **15.0** |

### Second Week of Sprint

| Day       | Activity                                      | Hours |
|-----------|-----------------------------------------------|-------|
| Monday    | Core Refinements — implemented bit-level I/O (BitWriter/BitReader), added tree serialization | 2.5   |
| Tuesday   | CLI Plan — designed compressed file format header, planned compress/decompress pipeline flow | 1.5   |
| Wednesday | CLI Implementation — integrated full compress and decompress pipelines, connected all modules | 2.5   |
| Thursday  | CLI Tests — tested compress mode with text files, verified header correctness, checked edge cases | 2.0   |
| Friday    | Integration Tests — round-trip testing (compress then decompress), tested with binary files, diff verification | 2.0   |
| Saturday  | Final Tests — stress tested with large files, cleaned up code, wrote README and documentation | 2.0   |
| **Subtotal** |                                             | **12.5** |

## Total

| Week     | Hours |
|----------|-------|
| Week 1   | 15.0  |
| Week 2   | 12.5  |
| **TOTAL**| **27.5** |

## Risks and Mitigations

| Risk | Action Plan |
|------|-------------|
| Understanding compression algorithms | Tested concepts in familiar languages (Python) first before implementing in C++ |
| Over-planning or having an unrealistic plan | Focused on simple, incremental plans — built and tested one module at a time |
