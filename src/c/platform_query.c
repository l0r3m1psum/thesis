#define CPUID(func, ax, bx, cx, dx)                  \
	__asm__ volatile (                               \
		"cpuid" :                                    \
		"=a" (ax), "=b" (bx), "=c" (cx), "=d" (dx) : \
		"a" (func)                                   \
	)

#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>
#include <inttypes.h>

/* https://softpixel.com/~cwright/programming/simd/cpuid.php
 * https://wiki.osdev.org/Inline_Assembly
 * https://stackoverflow.com/questions/14283171/how-to-receive-l1-l2-l3-cache-size-using-cpuid-instruction-in-x86
 * https://www.intel.com/content/dam/www/public/us/en/documents/manuals/64-ia-32-architectures-software-developer-instruction-set-reference-manual-325383.pdf#page=292
 */

typedef struct {
	uint32_t max_input;
	char buf[13];
} cpu_name_t;

typedef struct {
	uint32_t ways;
	uint32_t partitions;
	uint32_t line_size;
	uint32_t sets;
} cache_info_t;

static cpu_name_t
get_cpu_name(void) {
	uint32_t a, b, c, d;
	CPUID(0, a, b, c, d);
	cpu_name_t name = {0};
	name.max_input = a;
	memcpy(name.buf, &b, sizeof b);
	memcpy(name.buf+4, &d, sizeof d);
	memcpy(name.buf+8, &c, sizeof c);
	return name;
}

static cache_info_t
get_cache_info(uint32_t cache_level) {
	assert(cache_level < 4);
	uint32_t a, b, c, d;
	__asm__ volatile (
		"mov %0, %%ecx\n"
		"mov $4, %%eax\n"
		"cpuid"
		: "=a" (a), "=b" (b), "=c" (c), "=d" (d)
		: "r" (cache_level)
	);
	uint32_t res = 0, S = 0;
	__asm__ volatile (
		"mov %%ebx, %0\nmov %%ecx, %1"
		: "=r" (res), "=r" (S)
	);
#define L_maks 0x000007ff /* [11:0] */
#define P_mask 0x001ff800 /* [21:12] */
#define W_mask 0xffe00000 /* [31:22] */
	static_assert(L_maks | P_mask | W_mask == 0xffffffff, "bad masks");
	uint32_t
		L =  res & L_maks,
		P =  (res & P_mask) >> 12,
		W =  (res & W_mask) >> 22;
#undef L_maks
#undef P_maks
#undef W_maks
	cache_info_t info = {0};
	info.line_size = L+1;
	info.partitions = P+1;
	info.ways = W+1;
	info.sets = S+1;
	return info;
}

static void
print_cache_info(cache_info_t info) {
	printf("Cache Size: %" PRIu32 "\n", (info.ways) * (info.partitions) * (info.line_size) * (info.sets));
	printf("System Coherency Line Size %" PRIu32 "\n", info.line_size);
	printf("Physical Line partitions %" PRIu32 "\n", info.partitions);
	printf("Ways of associativity %" PRIu32 "\n", info.ways);
	printf("Number of Sets %" PRIu32 "\n", info.sets);
}

#ifdef TEST
int
main(void) {
	printf("%s\n", get_cpu_name().buf);

	{
		uint32_t a, b, c, d;
		CPUID(1, a, b, c, d);
		uint32_t res = 0;
		__asm__ volatile ("mov %%ebx, %0" : "=r" (res));
		/* Extracting %bh */
		res >>= 8;
		res &= 0x0000000f;
		printf("cache line size: %" PRIu32 "\n", res*8);
	}

	for (uint32_t i = 0; i < 4; i++) {
		printf("L%d\n", i);
		cache_info_t info = get_cache_info(i);
		print_cache_info(info);
	}

	return 0;
}
#endif
