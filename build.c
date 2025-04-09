#define IMPLEMENT_BUILD_H
#define BUILD_ITSELF
#include "build.h"
#include <assert.h>
#include <nmmintrin.h>

const char *build_bin = "build";
const char *build_source = "build.c";

#define HASHMAP_SIZE 1024 // Size of the hashmap
#define NUM_OPERATIONS 10000000 // Number of operations to perform

void log_bench_result(clock_t start, clock_t end, const char *name, size_t size);

int main(int argc, char **argv)
{
	// generic ordered map
	typedef struct {
		uint32_t l, r;
	} key_t;

	typedef struct {
		key_t key;
		int value;
	} pair_t;

	pair_t *map = NULL;
	size_t psz = sizeof(pair_t);
	map = init_hm(map, HASHMAP_SIZE, sizeof(pair_t), sse_hash, cmp_hash, 32);

	clock_t start, end;

	INFO("Insertion benchmark");
	// Insertion benchmark
	start = clock();
	for (int i = 0; i < NUM_OPERATIONS; i++) {
		pair_t kv = { 0 };
		kv.key.l = i;
		kv.key.r = i + 1;
		kv.value = i;
		hm_put(map, kv);
	}
	end = clock();
	log_bench_result(start, end, "ops", psz);

	INFO("Lookup benchmark");
	// Lookup benchmark
	start = clock();
	for (int i = 0; i < NUM_OPERATIONS; i++) {
		pair_t kv = { 0 };
		kv.key.l = i;
		kv.key.r = i + 1;
		int64_t fi = hm_geti(map, kv);
		// printf("%d - %d\n", map[fi].value, i);
		assert(fi != -1);
		assert(map[fi].value == i);
	}
	end = clock();
	log_bench_result(start, end, "lookups", psz);

	INFO("False lookup benchmark");
	start = clock();
	for (int i = 0; i < 1000000; i++) {
		pair_t kv = { 0 };
		kv.key.l = i + 1;
		kv.key.r = 0;
		int64_t fi = hm_geti(map, kv);
		if (fi != -1) printf("index = %ld\n", fi);
	}
	end = clock();
	log_bench_result(start, end, "lookups", psz);

	// execute shell commands
	execute("exit");

	return 0;
}

void log_bench_result(clock_t start, clock_t end, const char *name, size_t size)
{
	double cpu_time_used = ((double)(end - start)) / CLOCKS_PER_SEC;
	double ops_per_sec = NUM_OPERATIONS / cpu_time_used;
	double avg_time_per_op = cpu_time_used / NUM_OPERATIONS;

	INFO("%d %s in %f secs, %.2f ns/op, %.0f op/sec, %zu bytes/op", NUM_OPERATIONS, name, cpu_time_used, avg_time_per_op * 1e9, ops_per_sec, size);
}
