#include "slab_alloc.h"
#define __STDC_WANT_LIB_EXT1__ 1
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#ifndef _WIN32
#define SZ_FMT "%lu"
#else
#define WIN32_LEAN_AND_MEAN 1
#define STRICT 1
#include <windows.h>
#define SZ_FMT "%zu"
#endif

struct foo {
	int a;
	long b;
	char c;
};

enum { buffer_size = 0xFF };

typedef struct foo *(*foo_arr_ptr)[buffer_size];

// 
static inline void
fast_lot_slab_alloc(const unsigned size, struct foo* buffer[const size], struct slab_cache *cache) {

	for(size_t i = 0; i < size; ++i) {
		buffer[i] = slab_cache_alloc(cache);
	}

	for(size_t i = 0; i < size; ++i) {
		slab_cache_dealloc(cache, buffer[i]);
	}
}

static inline void
random_lot_slab_alloc(const unsigned size, struct foo* buffer[const size], struct slab_cache *cache) {
	
	size_t allocd = 0;
#ifdef __STDC_LIB_EXT1__
	errno_t err = memset_s(buffer, size, 0, sizeof(struct foo * [size]));
	assert(err == 0);
#else
#ifndef _WIN32
	void * err = memset(buffer, 0, sizeof(struct foo* [size]));
#else // _WIN32
	void * err = SecureZeroMemory(buffer, sizeof(struct foo* [size]));
	assert(err != 0);
#endif
#endif 

	do {
		size_t index = rand() % size;
		if(buffer[index] == NULL) {
			buffer[index] = slab_cache_alloc(cache);
			++allocd;
		}
	} while(allocd < size);

	do {
		--allocd;
		slab_cache_dealloc(cache, buffer[allocd]);
	} while(allocd != 0);
}

int main( const int argc,	char * argv[const argc])
{
	// size_t const size = 0xF; // 65536;
	// struct foo** const buffer = malloc(sizeof(*buffer) * size);

	foo_arr_ptr buffer = calloc(1, sizeof(*buffer)); // malloc(sizeof(*buffer) * size);
	
	assert(buffer);

	static struct slab_cache cache = {};

	srand(time(NULL));

	slab_cache_init(&cache, sizeof(struct foo), _Alignof(struct foo));

	printf("\n\n%s:\n\n Cache size:" SZ_FMT " stride:" SZ_FMT " max:" SZ_FMT " %p <-> %p\n",
		argv[0],
		cache.size, cache.stride, cache.slabmax,
		cache.front, cache.back);

	fast_lot_slab_alloc(buffer_size, *buffer, &cache);
	random_lot_slab_alloc(buffer_size, *buffer, &cache);

	slab_cache_deinit(&cache);

	free(buffer);

	printf("\n\n%s:\n\n DONE", argv[0]);
		return 0;
}

