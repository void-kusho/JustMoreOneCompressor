/*
 * Huffman File Compressor - Single-file C++ CLI
 *
 * Compresses and decompresses files using Huffman coding.
 * Uses ONLY stack-based arrays — no pointers, no heap allocation.
 *
 * Usage:
 *   ./huffman compress <input_file> <output_file>
 *   ./huffman decompress <input_file> <output_file>
 *
 * Author: Student
 * Course: CSE 310 - BYU
 */

#include <cstdio>
#include <cstdlib>
#include <cstring>

/* ------------------------------------------------------------------ */
/*  Constants — all memory is statically / stack-allocated             */
/* ------------------------------------------------------------------ */

static constexpr int ALPHABET      = 256;   /* one entry per byte value       */
static constexpr int MAX_NODES     = 512;   /* 2*ALPHABET is enough for tree  */
static constexpr int MAX_CODE_LEN  = 256;   /* longest possible Huffman code  */
static constexpr int BUF_SIZE      = 4096;  /* I/O buffer size                */
static constexpr int HEADER_SIZE   = 1024;  /* bytes reserved for tree header */

/* ------------------------------------------------------------------ */
/*  Huffman tree stored in parallel arrays (index-based, no pointers)  */
/* ------------------------------------------------------------------ */

struct TreeNode {
    int freq;          /* frequency / combined weight          */
    int symbol;        /* byte value (-1 for internal nodes)   */
    int left;          /* index of left child  (-1 = none)     */
    int right;         /* index of right child (-1 = none)     */
    bool used;         /* whether this slot is occupied         */
};

/* Code entry for each byte value */
struct CodeEntry {
    int bits[MAX_CODE_LEN];  /* each element is 0 or 1          */
    int length;              /* number of valid bits             */
};

/* ------------------------------------------------------------------ */
/*  Global stack arrays (avoid heap entirely)                          */
/* ------------------------------------------------------------------ */

static TreeNode tree[MAX_NODES];
static CodeEntry codes[ALPHABET];
static int freq_table[ALPHABET];

/* ------------------------------------------------------------------ */
/*  Utility: reset the tree                                            */
/* ------------------------------------------------------------------ */

/* Reset all tree nodes to unused state */
void reset_tree() {
    for (int i = 0; i < MAX_NODES; ++i) {
        tree[i].freq   = 0;
        tree[i].symbol = -1;
        tree[i].left   = -1;
        tree[i].right  = -1;
        tree[i].used   = false;
    }
}

/* ------------------------------------------------------------------ */
/*  Utility: find the two nodes with the smallest frequencies          */
/* ------------------------------------------------------------------ */

/* Return indices of the two lowest-frequency unused nodes via out params */
void find_two_smallest(int node_count, int& min1, int& min2) {
    min1 = -1;
    min2 = -1;

    /* Find the smallest */
    for (int i = 0; i < node_count; ++i) {
        if (!tree[i].used) continue;
        if (min1 == -1 || tree[i].freq < tree[min1].freq) {
            min1 = i;
        }
    }

    /* Find the second smallest */
    for (int i = 0; i < node_count; ++i) {
        if (!tree[i].used) continue;
        if (i == min1) continue;
        if (min2 == -1 || tree[i].freq < tree[min2].freq) {
            min2 = i;
        }
    }
}

/* ------------------------------------------------------------------ */
/*  Build the Huffman tree from the frequency table                    */
/* ------------------------------------------------------------------ */

/*
 * Populates the tree[] array.  Returns the index of the root node,
 * or -1 if there is nothing to compress.
 */
int build_huffman_tree() {
    reset_tree();

    int node_count = 0;

    /* Create leaf nodes for every byte that appears */
    for (int i = 0; i < ALPHABET; ++i) {
        if (freq_table[i] > 0) {
            tree[node_count].freq   = freq_table[i];
            tree[node_count].symbol = i;
            tree[node_count].left   = -1;
            tree[node_count].right  = -1;
            tree[node_count].used   = true;
            ++node_count;
        }
    }

    /* Edge case: empty file or file with no data */
    if (node_count == 0) return -1;

    /* Edge case: only one unique byte — add a dummy node so the tree
       has at least two leaves and codes are well-defined. */
    if (node_count == 1) {
        tree[node_count].freq   = 1;
        tree[node_count].symbol = -1;
        tree[node_count].left   = -1;
        tree[node_count].right  = -1;
        tree[node_count].used   = true;
        ++node_count;
    }

    /* Repeatedly merge the two smallest nodes */
    while (true) {
        int min1, min2;
        find_two_smallest(node_count, min1, min2);

        if (min2 == -1) break;  /* only one node left — that's the root */

        int parent = node_count++;
        tree[parent].freq   = tree[min1].freq + tree[min2].freq;
        tree[parent].symbol = -1;
        tree[parent].left   = min1;
        tree[parent].right  = min2;
        tree[parent].used   = true;

        /* Mark children as "consumed" so they won't be picked again */
        tree[min1].used = false;
        tree[min2].used = false;
    }

    /* The root is the only node still marked used */
    for (int i = 0; i < node_count; ++i) {
        if (tree[i].used) return i;
    }

    return -1;  /* should never reach here */
}

/* ------------------------------------------------------------------ */
/*  Generate Huffman codes by traversing the tree (recursive, stack)   */
/* ------------------------------------------------------------------ */

/*
 * Walk the tree depth-first.  `path` accumulates the bit sequence,
 * and `depth` tracks how many bits we have so far.
 */
void generate_codes(int node_idx, int path[], int depth) {
    if (node_idx == -1) return;

    /* Leaf node — store the code */
    if (tree[node_idx].left == -1 && tree[node_idx].right == -1
        && tree[node_idx].symbol >= 0) {
        int sym = tree[node_idx].symbol;
        codes[sym].length = depth;
        for (int i = 0; i < depth; ++i) {
            codes[sym].bits[i] = path[i];
        }
        return;
    }

    /* Traverse left (append 0) */
    path[depth] = 0;
    generate_codes(tree[node_idx].left, path, depth + 1);

    /* Traverse right (append 1) */
    path[depth] = 1;
    generate_codes(tree[node_idx].right, path, depth + 1);
}

/* ------------------------------------------------------------------ */
/*  Serialize the Huffman tree into a byte buffer for the file header  */
/* ------------------------------------------------------------------ */

/*
 * Writes a pre-order traversal of the tree into `buf`.
 * Format per node:
 *   1 byte: 0x00 = internal, 0x01 = leaf
 *   if leaf: 1 byte = symbol value
 * Returns the number of bytes written.
 */
int serialize_tree(int node_idx, unsigned char buf[], int offset) {
    if (node_idx == -1) return offset;

    int pos = offset;

    /* Check if leaf */
    bool is_leaf = (tree[node_idx].left == -1 && tree[node_idx].right == -1);

    if (is_leaf) {
        buf[pos++] = 0x01;  /* leaf marker */
        buf[pos++] = static_cast<unsigned char>(tree[node_idx].symbol);
    } else {
        buf[pos++] = 0x00;  /* internal node marker */
        pos = serialize_tree(tree[node_idx].left,  buf, pos);
        pos = serialize_tree(tree[node_idx].right, buf, pos);
    }

    return pos;
}

/* ------------------------------------------------------------------ */
/*  Deserialize the Huffman tree from a byte buffer                    */
/* ------------------------------------------------------------------ */

/*
 * Reads a pre-order traversal from `buf` starting at `*offset`.
 * Returns the index of the reconstructed node.
 */
int deserialize_tree(const unsigned char buf[], int& offset, int& node_count) {
    int idx = node_count++;
    tree[idx].left  = -1;
    tree[idx].right = -1;
    tree[idx].used  = true;

    unsigned char marker = buf[offset++];

    if (marker == 0x01) {
        /* Leaf node */
        tree[idx].symbol = buf[offset++];
        tree[idx].freq   = 1;  /* not needed for decompression */
    } else {
        /* Internal node */
        tree[idx].symbol = -1;
        tree[idx].freq   = 0;
        tree[idx].left   = deserialize_tree(buf, offset, node_count);
        tree[idx].right  = deserialize_tree(buf, offset, node_count);
    }

    return idx;
}

/* ------------------------------------------------------------------ */
/*  Bit writer — accumulates bits and flushes full bytes to file       */
/* ------------------------------------------------------------------ */

struct BitWriter {
    FILE* file;
    unsigned char buffer;   /* accumulated bits */
    int bit_count;          /* how many bits are in buffer (0-7) */
    long long total_bits;   /* total bits written (stored in header) */
};

/* Initialize the bit writer */
void bit_writer_init(BitWriter& bw, FILE* f) {
    bw.file       = f;
    bw.buffer     = 0;
    bw.bit_count  = 0;
    bw.total_bits = 0;
}

/* Write a single bit */
void bit_writer_write(BitWriter& bw, int bit) {
    bw.buffer = (bw.buffer << 1) | (bit & 1);
    bw.bit_count++;
    bw.total_bits++;

    if (bw.bit_count == 8) {
        fputc(bw.buffer, bw.file);
        bw.buffer    = 0;
        bw.bit_count = 0;
    }
}

/* Flush remaining bits (pad with zeros) */
void bit_writer_flush(BitWriter& bw) {
    if (bw.bit_count > 0) {
        bw.buffer <<= (8 - bw.bit_count);  /* pad with zeros */
        fputc(bw.buffer, bw.file);
        bw.buffer    = 0;
        bw.bit_count = 0;
    }
}

/* ------------------------------------------------------------------ */
/*  Bit reader — reads bytes from file and yields individual bits      */
/* ------------------------------------------------------------------ */

struct BitReader {
    FILE* file;
    unsigned char buffer;   /* current byte */
    int bit_count;          /* how many bits consumed from buffer */
    bool eof;               /* end-of-file flag */
};

/* Initialize the bit reader */
void bit_reader_init(BitReader& br, FILE* f) {
    br.file      = f;
    br.buffer    = 0;
    br.bit_count = 8;  /* force first read */
    br.eof       = false;
}

/* Read a single bit; returns -1 on EOF */
int bit_reader_read(BitReader& br) {
    if (br.bit_count == 8) {
        int c = fgetc(br.file);
        if (c == EOF) {
            br.eof = true;
            return -1;
        }
        br.buffer    = static_cast<unsigned char>(c);
        br.bit_count = 0;
    }

    int bit = (br.buffer >> (7 - br.bit_count)) & 1;
    br.bit_count++;
    return bit;
}

/* ------------------------------------------------------------------ */
/*  Count frequencies by reading the entire input file                 */
/* ------------------------------------------------------------------ */

/* Returns total number of bytes read */
long long count_frequencies(const char* filename) {
    /* Reset frequency table */
    for (int i = 0; i < ALPHABET; ++i) {
        freq_table[i] = 0;
    }

    FILE* f = fopen(filename, "rb");
    if (!f) {
        fprintf(stderr, "Error: cannot open file '%s'\n", filename);
        return -1;
    }

    unsigned char buf[BUF_SIZE];
    long long total = 0;
    size_t n;

    while ((n = fread(buf, 1, BUF_SIZE, f)) > 0) {
        for (size_t i = 0; i < n; ++i) {
            freq_table[buf[i]]++;
        }
        total += n;
    }

    fclose(f);
    return total;
}

/* ------------------------------------------------------------------ */
/*  COMPRESS                                                             */
/* ------------------------------------------------------------------ */

/*
 * File format:
 *   [4 bytes] original file size (little-endian, 64-bit stored in 8 bytes)
 *   [4 bytes] tree header length (little-endian)
 *   [tree header bytes] serialized Huffman tree
 *   [4 bytes] total compressed bit count (little-endian)
 *   [compressed data ...]
 */

int compress_file(const char* input, const char* output) {
    printf("Compressing '%s' -> '%s'\n", input, output);

    /* Step 1: count frequencies */
    long long file_size = count_frequencies(input);
    if (file_size < 0) return 1;

    if (file_size == 0) {
        printf("Input file is empty, nothing to compress.\n");
        return 1;
    }

    printf("  File size: %lld bytes\n", file_size);
    printf("  Counting frequencies... done.\n");

    /* Step 2: build Huffman tree */
    int root = build_huffman_tree();
    if (root == -1) {
        fprintf(stderr, "Error: failed to build Huffman tree.\n");
        return 1;
    }
    printf("  Huffman tree built (root at index %d).\n", root);

    /* Step 3: generate codes */
    int path[MAX_CODE_LEN];
    for (int i = 0; i < ALPHABET; ++i) {
        codes[i].length = 0;
    }
    generate_codes(root, path, 0);
    printf("  Codes generated.\n");

    /* Step 4: serialize tree */
    unsigned char tree_buf[HEADER_SIZE];
    int tree_len = serialize_tree(root, tree_buf, 0);
    printf("  Serialized tree: %d bytes.\n", tree_len);

    /* Step 5: open output file */
    FILE* out = fopen(output, "wb");
    if (!out) {
        fprintf(stderr, "Error: cannot create output file '%s'\n", output);
        return 1;
    }

    /* Write header: file_size (8 bytes, little-endian) */
    for (int i = 0; i < 8; ++i) {
        fputc(static_cast<int>((file_size >> (i * 8)) & 0xFF), out);
    }

    /* Write header: tree length (4 bytes, little-endian) */
    for (int i = 0; i < 4; ++i) {
        fputc(static_cast<int>((tree_len >> (i * 8)) & 0xFF), out);
    }

    /* Write header: serialized tree */
    fwrite(tree_buf, 1, tree_len, out);

    /* Step 6: compress the file */
    FILE* in = fopen(input, "rb");
    if (!in) {
        fprintf(stderr, "Error: cannot reopen input file.\n");
        fclose(out);
        return 1;
    }

    BitWriter bw;
    bit_writer_init(bw, out);

    unsigned char read_buf[BUF_SIZE];
    size_t n;
    long long bytes_written = 0;

    while ((n = fread(read_buf, 1, BUF_SIZE, in)) > 0) {
        for (size_t i = 0; i < n; ++i) {
            int sym = read_buf[i];
            for (int b = 0; b < codes[sym].length; ++b) {
                bit_writer_write(bw, codes[sym].bits[b]);
            }
        }
        bytes_written += n;
        if (bytes_written % 65536 == 0) {
            printf("  Progress: %lld / %lld bytes\r", bytes_written, file_size);
        }
    }

    bit_writer_flush(bw);
    fclose(in);

    /* Write total bit count at end of file as metadata */
    long long total_bits = bw.total_bits;
    for (int i = 0; i < 8; ++i) {
        fputc(static_cast<int>((total_bits >> (i * 8)) & 0xFF), out);
    }

    fclose(out);

    printf("\n  Compressed: %lld bits written.\n", total_bits);
    printf("  Compression ratio: %.2f%%\n",
           (static_cast<double>(total_bits / 8.0) / file_size) * 100.0);

    return 0;
}

/* ------------------------------------------------------------------ */
/*  DECOMPRESS                                                           */
/* ------------------------------------------------------------------ */

int decompress_file(const char* input, const char* output) {
    printf("Decompressing '%s' -> '%s'\n", input, output);

    FILE* in = fopen(input, "rb");
    if (!in) {
        fprintf(stderr, "Error: cannot open file '%s'\n", input);
        return 1;
    }

    /* Step 1: read file_size (8 bytes, little-endian) */
    long long file_size = 0;
    for (int i = 0; i < 8; ++i) {
        int b = fgetc(in);
        if (b == EOF) {
            fprintf(stderr, "Error: invalid compressed file.\n");
            fclose(in);
            return 1;
        }
        file_size |= (static_cast<long long>(b) << (i * 8));
    }

    printf("  Original file size: %lld bytes\n", file_size);

    /* Step 2: read tree length (4 bytes, little-endian) */
    int tree_len = 0;
    for (int i = 0; i < 4; ++i) {
        int b = fgetc(in);
        if (b == EOF) {
            fprintf(stderr, "Error: invalid compressed file.\n");
            fclose(in);
            return 1;
        }
        tree_len |= (b << (i * 8));
    }

    printf("  Tree header: %d bytes\n", tree_len);

    /* Step 3: read serialized tree */
    unsigned char tree_buf[HEADER_SIZE];
    if (fread(tree_buf, 1, tree_len, in) != static_cast<size_t>(tree_len)) {
        fprintf(stderr, "Error: failed to read tree header.\n");
        fclose(in);
        return 1;
    }

    /* Step 4: deserialize tree */
    reset_tree();
    int node_count = 0;
    int offset = 0;
    int root = deserialize_tree(tree_buf, offset, node_count);
    printf("  Tree deserialized (%d nodes, root at %d).\n", node_count, root);

    /* Step 5: open output file */
    FILE* out = fopen(output, "wb");
    if (!out) {
        fprintf(stderr, "Error: cannot create output file '%s'\n", output);
        fclose(in);
        return 1;
    }

    /* Step 6: decompress by walking the tree bit by bit */
    BitReader br;
    bit_reader_init(br, in);

    long long bytes_out = 0;
    int current = root;

    while (bytes_out < file_size) {
        int bit = bit_reader_read(br);
        if (bit == -1) {
            fprintf(stderr, "Error: unexpected end of compressed data.\n");
            break;
        }

        if (bit == 0) {
            current = tree[current].left;
        } else {
            current = tree[current].right;
        }

        /* Check if we reached a leaf */
        if (current == -1) {
            fprintf(stderr, "Error: invalid Huffman code encountered.\n");
            break;
        }

        if (tree[current].left == -1 && tree[current].right == -1) {
            /* Leaf node — output the symbol */
            fputc(tree[current].symbol, out);
            bytes_out++;
            current = root;  /* restart from root for next symbol */

            if (bytes_out % 65536 == 0) {
                printf("  Progress: %lld / %lld bytes\r", bytes_out, file_size);
            }
        }
    }

    fclose(out);
    fclose(in);

    printf("\n  Decompressed: %lld bytes written.\n", bytes_out);

    if (bytes_out != file_size) {
        fprintf(stderr, "Warning: output size (%lld) != expected (%lld)\n",
                bytes_out, file_size);
        return 1;
    }

    return 0;
}

/* ------------------------------------------------------------------ */
/*  Print usage information                                            */
/* ------------------------------------------------------------------ */

void print_usage(const char* prog) {
    printf("Huffman File Compressor\n");
    printf("Usage:\n");
    printf("  %s compress   <input_file> <output_file>\n", prog);
    printf("  %s decompress <input_file> <output_file>\n", prog);
    printf("\nExamples:\n");
    printf("  %s compress document.txt document.huff\n", prog);
    printf("  %s decompress document.huff document_restored.txt\n", prog);
}

/* ------------------------------------------------------------------ */
/*  main                                                                 */
/* ------------------------------------------------------------------ */

int main(int argc, char* argv[]) {
    if (argc != 4) {
        print_usage(argv[0]);
        return 1;
    }

    const char* mode   = argv[1];
    const char* input  = argv[2];
    const char* output = argv[3];

    if (strcmp(mode, "compress") == 0) {
        return compress_file(input, output);
    } else if (strcmp(mode, "decompress") == 0) {
        return decompress_file(input, output);
    } else {
        fprintf(stderr, "Error: unknown mode '%s'\n", mode);
        print_usage(argv[0]);
        return 1;
    }
}
