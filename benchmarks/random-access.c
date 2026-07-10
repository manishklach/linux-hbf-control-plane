// SPDX-License-Identifier: GPL-2.0
/*
 * random-access.c  —  Benchmark HBF prefetch on random access
 *
 * Usage:
 *   ./random-access <size_mb> [hot_node] [warm_node]
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <numa.h>
#include <numaif.h>
#include <time.h>
#include <linux/types.h>

#define DEV_PATH "/dev/hbfctl"

static int dev_fd = -1;
static int open_dev(void)
{
	if (dev_fd >= 0) return 0;
	dev_fd = open(DEV_PATH, O_RDWR);
	return dev_fd >= 0 ? 0 : -1;
}

static unsigned long time_ns(void)
{
	struct timespec ts;
	clock_gettime(CLOCK_MONOTONIC, &ts);
	return ts.tv_sec * 1000000000UL + ts.tv_nsec;
}

int main(int argc, char **argv)
{
	size_t size_mb = argc > 1 ? atol(argv[1]) : 64;
	int hot_node = argc > 2 ? atoi(argv[2]) : 0;
	int warm_node = argc > 3 ? atoi(argv[3]) : 1;
	size_t size = size_mb * 1024 * 1024;
	size_t pages = size / 4096;
	char *buf;
	int i;

	if (!numa_available()) {
		fprintf(stderr, "NUMA not available\n");
		return 1;
	}

	buf = numa_alloc_onnode(size, warm_node);
	if (!buf) {
		perror("numa_alloc_onnode");
		return 1;
	}
	memset(buf, 0, size);

	if (open_dev() < 0) {
		printf("{\"error\": \"cannot open %s\"}\n", DEV_PATH);
		return 1;
	}

	// Generate random access pattern
	long *indices = malloc(pages * sizeof(long));
	for (i = 0; i < pages; i++)
		indices[i] = i;
	for (i = pages - 1; i > 0; i--) {
		int j = rand() % (i + 1);
		long tmp = indices[i];
		indices[i] = indices[j];
		indices[j] = tmp;
	}

	struct hbf_user_hint hint = {
		.version = 1,
		.op = 1,
		.start = (unsigned long)buf,
		.len = size,
		.target_nid = hot_node,
		.deadline_ns = time_ns() + 500000000,
		.user_tag = 1,
	};

	unsigned long t_submit = time_ns();
	int ret = ioctl(dev_fd, HBF_IOC_SUBMIT, &hint);
	unsigned long t_submitted = time_ns();

	if (ret < 0) {
		printf("{\"error\": \"SUBMIT failed: %d\"}\n", ret);
		return 1;
	}

	usleep(200000);

	// Touch random pages
	unsigned long *fault_times = malloc(pages * sizeof(unsigned long));
	for (i = 0; i < pages; i++) {
		volatile char tmp;
		unsigned long t = time_ns();
		tmp = buf[indices[i] * 4096];
		fault_times[i] = time_ns() - t;
	}

	qsort(fault_times, pages, sizeof(unsigned long),
	      (void *)(int (*)(const void *, const void *)) \
	      (long long (*)(const void *, const void *)) \
	      [](const void *a, const void *b) {
		      return (*(unsigned long *)a > *(unsigned long *)b) -
			     (*(unsigned long *)a < *(unsigned long *)b);
	      });

	unsigned long p50 = fault_times[pages / 2];
	unsigned long p95 = fault_times[pages * 95 / 100];
	unsigned long p99 = fault_times[pages * 99 / 100];

	printf("{\n");
	printf("  \"benchmark\": \"random-access\",\n");
	printf("  \"size_mb\": %zu,\n", size_mb);
	printf("  \"hot_node\": %d,\n", hot_node);
	printf("  \"warm_node\": %d,\n", warm_node);
	printf("  \"submit_latency_ns\": %lu,\n", t_submitted - t_submit);
	printf("  \"p50_fault_ns\": %lu,\n", p50);
	printf("  \"p95_fault_ns\": %lu,\n", p95);
	printf("  \"p99_fault_ns\": %lu,\n", p99);
	printf("  \"pages\": %zu\n", pages);
	printf("}\n");

	free(indices);
	free(fault_times);
	close(dev_fd);
	return 0;
}
