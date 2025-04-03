#define IMPLEMENT_BUILD_H
#define BUILD_ITSELF
#include "build.h"

const char *build_bin = "build";
const char *build_source = "build.c";

#define HASHMAP_SIZE 1000000 // Size of the hashmap
#define NUM_OPERATIONS 500000 // Number of operations to perform

void log_bench_result(clock_t start, clock_t end, const char *name);

int main(int argc, char **argv)
{
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

	for (int i = 0; i < darray_get_length(ints); ++i)
		INFO("%d", ints[i]);

	// generic ordered map
	typedef struct {
		char key[20];
		int value;
	} pair_t;

	INFO("Insertion benchmark");
	for (size_t i = 0; i < 10; ++i)
	{
		pair_t *map = NULL;
		map = init_hm(map, sizeof(pair_t), HASHMAP_SIZE, MM86128, cmp_hash, 32);

		clock_t start, end;

		// Insertion benchmark
		start = clock();
		for (int i = 0; i < NUM_OPERATIONS; i++) {
			pair_t kv = { 0 };
			snprintf(kv.key, sizeof(kv.key), "key%d", i);
			hm_put(map, kv);
		}
		end = clock();
		log_bench_result(start, end, "ops");

		/*
		// Lookup benchmark
		start = clock();
		for (int i = 0; i < NUM_OPERATIONS; i++) {
			pair_t kv = { 0 };
			snprintf(kv.key, sizeof(kv.key), "key%d", i);
			hm_get(map, (&kv));
		}
		end = clock();
		log_bench_result(start, end, "lookups");
		*/

		hm_free(map);
	}

	INFO("Lookup benchmark");
	for (size_t i = 0; i < 10; ++i)
	{
		pair_t *map = NULL;
		map = init_hm(map, sizeof(pair_t), HASHMAP_SIZE, fnv_1a_hash, cmp_hash, 32);

		clock_t start, end;

		// Lookup benchmark
		start = clock();
		for (int i = 0; i < NUM_OPERATIONS; i++) {
			pair_t kv = { 0 };
			snprintf(kv.key, sizeof(kv.key), "key%d", i);
			hm_get(map, (&kv));
		}
		end = clock();
		log_bench_result(start, end, "lookups");

		hm_free(map);
	}

	// execute shell commands
	execute("exit");

	return 0;
}

void log_bench_result(clock_t start, clock_t end, const char *name)
{
	double cpu_time_used = ((double)(end - start)) / CLOCKS_PER_SEC;
	double ops_per_sec = NUM_OPERATIONS / cpu_time_used;
	double avg_time_per_op = cpu_time_used / NUM_OPERATIONS;
	double bytes_per_op = (sizeof(char*) * NUM_OPERATIONS + sizeof(int) * NUM_OPERATIONS) / (double)NUM_OPERATIONS;

	INFO("%d %s in %f secs, %.2f ns/op, %.0f op/sec, %.2f bytes/op", NUM_OPERATIONS, name, cpu_time_used, avg_time_per_op * 1e9, ops_per_sec, bytes_per_op);
}
