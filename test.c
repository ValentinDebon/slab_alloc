#include "slab_alloc.h"

#include <stdio.h>
#include <stdlib.h>

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

int
main(int argc,
	char **argv) {
	size_t const size = 1048576;
	struct foo ** const buffer = malloc(sizeof(*buffer) * size);
	struct slab_cache cache;

	slab_cache_init(&cache, sizeof(struct foo), _Alignof(struct foo));

	printf("Cache size:%lu stride:%lu max:%lu %p <-> %p\n",
		cache.size, cache.stride, cache.slabmax,
		cache.front, cache.back);

	fast_lot_slab_alloc(buffer, size, &cache);

	slab_cache_deinit(&cache);
	free(buffer);

	return 0;
}

