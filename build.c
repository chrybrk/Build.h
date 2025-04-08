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
	/*
	// Logging with info, warn and error
	INFO("Hello, world!");

	// string formating
	char *string = formate_string("hello from %s", build_bin);
	INFO("before = %s", string);

	// in-string replacement
	string = replace_char_in_string(string, ' ', ',');
	INFO("after replacement = %s", string);

	// generic dynamic array
	int *ints = NULL;
	ints = init_darray(ints, sizeof(int), 5);

	for (int i = 0; i <= 10; ++i)
		darray_push(ints, i * 10);

	for (int i = 0; i < darray_len(ints); ++i)
		INFO("%d", ints[i]);

	*/
	// generic ordered map
	typedef struct {
		size_t key;
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
		kv.key = i;
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
		kv.key = i;
		long int fi = hm_geti(map, kv);
		// printf("%d - %d\n", map[fi].value, i);
		assert(map[fi].value == i);
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
