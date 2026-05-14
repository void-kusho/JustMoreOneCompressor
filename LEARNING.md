# Discussion of Learning Strategies

## Module: Huffman File Compressor

### 1. Strategies Used

#### Active Recall and Practice
Rather than passively reading about Huffman coding, I implemented the algorithm from scratch. Each component (frequency counting, tree building, code generation, bit I/O) was written and tested independently before integration. This reinforced understanding of how the pieces connect.

#### Incremental Development
I built the compressor in stages:
1. Frequency counting — verified against known inputs
2. Tree construction — printed tree structure to validate
3. Code generation — compared codes against manual calculations
4. Serialization — verified tree round-trip (serialize then deserialize)
5. Full compression/decompression — tested with `diff` for byte-exact match

This approach made debugging manageable and ensured each layer was correct before adding complexity.

#### Constraint-Driven Learning
The "no heap, no pointers" constraint forced me to think differently about data structures. Instead of `new TreeNode` and pointer-based trees, I used parallel arrays with index-based navigation. This deepened my understanding of how data structures work at a lower level and how memory layout affects design.

#### Cross-Reference Learning
I consulted multiple sources to understand concepts:
- Wikipedia for the Huffman algorithm overview
- cppreference.com for C++ I/O functions (`fread`, `fgetc`, etc.)
- Stack Overflow for bit-packing techniques
- Textbook chapters on greedy algorithms and prefix codes

Comparing explanations from different sources helped solidify understanding.

### 2. Challenges and How I Overcame Them

#### Challenge: Bit-Level I/O
Writing individual bits to a file is not directly supported by standard I/O. I had to implement a buffer that accumulates bits and flushes full bytes.

**Solution**: Created `BitWriter` and `BitReader` structs that manage an 8-bit buffer and a bit counter. This is a common pattern in compression libraries.

#### Challenge: Tree Serialization
The Huffman tree must be stored in the compressed file so decompression can reconstruct it. Serializing a tree structure to a flat byte stream required careful design.

**Solution**: Used pre-order traversal with markers (0x00 for internal, 0x01 for leaf). This is unambiguous and easy to deserialize by following the same traversal pattern.

#### Challenge: No Dynamic Memory
The constraint of no heap allocation meant I had to size all arrays at compile time.

**Solution**: Used `static constexpr` constants and fixed-size arrays. The maximum alphabet size (256) and maximum tree nodes (512) are known bounds, so this was feasible.

### 3. What I Learned

- **Greedy algorithms**: Huffman coding is a classic greedy algorithm — always merging the two smallest frequencies produces an optimal prefix code.
- **Prefix codes**: No code is a prefix of another, which enables unambiguous decoding without delimiters.
- **Bit manipulation**: Working at the bit level (shifting, masking, packing) is essential for compression.
- **File formats**: Designing a binary file format requires careful attention to byte order, alignment, and metadata.
- **Testing methodology**: Round-trip testing (compress then decompress and compare) is the gold standard for compression software.

### 4. Future Improvements

- Add a canonical Huffman code option for smaller tree headers
- Implement adaptive Huffman coding for streaming compression
- Add support for larger files with chunked processing
- Benchmark against existing tools (gzip, xz)
