# Simplified Slab allocator

ANSI-C simplified slab allocator. Concepts taken from "The Slab Allocator: An Object-Caching Kernel Memory Allocator" By Jeff Bonwick.
Simplified because it doesn't support:
- Big allocations (bigger than pagesize / 8).
- Initialization and Deinitialization as described in the original paper.

## Build

	clang -ansi -pedantic -Wall -c slab_alloc.c

## Test

	clang -Wall -o test slab_alloc.c test.c
