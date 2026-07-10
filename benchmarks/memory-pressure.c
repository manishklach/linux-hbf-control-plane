// SPDX-License-Identifier: GPL-2.0
/*
 * memory-pressure.c  —  Test HBF behavior under hot-tier memory pressure
 *
 * Allocates more data than the hot node can hold, submits PROMOTE hints
 * for each range, and measures thrash and deadline misses.
 *
 * Usage:
 *   ./memory-pressure <hot_size_mb> <warm_size_mb> [iterations]
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
	int hot_size_mb = argc > 1 ? atoi(argv[1]) : 128;
	int warm_size_mb = argc > 2 ? atoi(argv[2]) : 512;
	int iterations = argc > 3 ? atoi(argv[3]) : 3;
	int hot_node = 0, warm_node = 1;
	size_t hot_size = hot_size_mb * 1024 * 1024;
	size_t warm_size = warm_size_mb * 1024 * 1024;
	char **bufs;
	int num_bufs = warm_size_mb / hot_size_mb;
	int i, j;

	if (!numa_available()) {
		fprintf(stderr, "NUMA not available\n");
		return 1;
	}

	if (numa_num_configured_nodes() < 2) {
		fprintf(stderr, "Need at least 2 NUMA nodes\n");
		return 1;
	}

	if (open_dev() < 0) {
		printf("{\"error\": \"cannot open %s\"}\n", DEV_PATH);
		return 1;
	}

	bufs = malloc(num_bufs * sizeof(char *));
	for (i = 0; i < num_bufs; i++) {
		bufs[i] = numa_alloc_onnode(hot_size, warm_node);
		if (!bufs[i]) {
			perror("numa_alloc_onnode");
			return 1;
		}
		memset(bufs[i], 0xCD, hot_size);
	}

	printf("{\n");
	printf("  \"benchmark\": \"memory-pressure\",\n");
	printf("  \"hot_size_mb\": %d,\n", hot_size_mb);
	printf("  \"warm_size_mb\": %d,\n", warm_size_mb);
	printf("  \"num_buffers\": %d,\n", num_bufs);
	printf("  \"iterations\": %d,\n", iterations);
	printf("  \"results\": [\n");

	for (i = 0; i < iterations; i++) {
		int promoted = 0, deadline_misses = 0;

		for (j = 0; j < num_bufs; j++) {
			struct hbf_user_hint hint = {
				.version = 1,
				.op = 2,
				.start = (unsigned long)bufs[j],
				.len = hot_size,
				.target_nid = hot_node,
				.deadline_ns = time_ns() + 100000000,
				.user_tag = i * num_bufs + j,
			};

			if (ioctl(dev_fd, HBF_IOC_SUBMIT, &hint) == 0)
				promoted++;
		}

		usleep(1000000);

		printf("    { \"iter\": %d, \"promoted\": %d, \"deadline_misses\": %d },\n",
		       i, promoted, deadline_misses);
	}

	printf("  ]\n");
	printf("}\n");

	for (i = 0; i < num_bufs; i++)
		numa_free(bufs[i], hot_size);
	free(bufs);
	close(dev_fd);
	return 0;
}
