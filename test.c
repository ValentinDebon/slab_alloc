#include "slab_alloc.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

struct foo {
	int a;
	long b;
	char c;
};

static void
fast_lot_slab_alloc(struct foo **buffer, size_t size, struct slab_cache *cache) {

	for(size_t i = 0; i < size; ++i) {
		buffer[i] = slab_cache_alloc(cache);
	}

	for(size_t i = 0; i < size; ++i) {
		slab_cache_dealloc(cache, buffer[i]);
	}
}

static void
random_lot_slab_alloc(struct foo **buffer, size_t size, struct slab_cache *cache) {
	size_t allocd = 0;
	memset(buffer, 0, size * sizeof(*buffer));

	do {
		size_t index = rand() % size;
		if(buffer[index] == NULL) {
			buffer[index] = slab_cache_alloc(cache);
			++allocd;
		}
	} while(allocd < size);

	do {
		size_t index = rand() % size;
		if(buffer[index] != NULL) {
			slab_cache_dealloc(cache, buffer[index]);
			buffer[index] = NULL;
			--allocd;
		}
	} while(allocd != 0);
}

int
main(int argc,
	char **argv) {
	size_t const size = 65536;
	struct foo ** const buffer = malloc(sizeof(*buffer) * size);
	struct slab_cache cache;

	srand(time(NULL));

	slab_cache_init(&cache, sizeof(struct foo), _Alignof(struct foo));

	printf("Cache size:%lu stride:%lu max:%lu %p <-> %p\n",
		cache.size, cache.stride, cache.slabmax,
		cache.front, cache.back);

	fast_lot_slab_alloc(buffer, size, &cache);
	random_lot_slab_alloc(buffer, size, &cache);

	slab_cache_deinit(&cache);
	free(buffer);

	return 0;
}

