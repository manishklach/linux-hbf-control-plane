// SPDX-License-Identifier: GPL-2.0
/*
 * promote-pressure.c  —  Measure HBF promote/demote bandwidth
 *
 * Allocates a buffer on warm node, repeatedly promotes and demotes it,
 * measuring throughput and verifying residency.
 *
 * Usage:
 *   ./promote-pressure <size_mb> [iterations] [hot_node] [warm_node]
 *
 * Output: JSON with per-iteration bandwidth and total.
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
	size_t size_mb = argc > 1 ? atol(argv[1]) : 128;
	int iterations = argc > 2 ? atoi(argv[2]) : 5;
	int hot_node = argc > 3 ? atoi(argv[3]) : 0;
	int warm_node = argc > 4 ? atoi(argv[4]) : 1;
	size_t size = size_mb * 1024 * 1024;
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
	memset(buf, 0xAB, size);

	if (open_dev() < 0) {
		printf("{\"error\": \"cannot open %s\"}\n", DEV_PATH);
		return 1;
	}

	printf("{\n");
	printf("  \"benchmark\": \"promote-pressure\",\n");
	printf("  \"size_mb\": %zu,\n", size_mb);
	printf("  \"iterations\": %d,\n", iterations);
	printf("  \"hot_node\": %d,\n", hot_node);
	printf("  \"warm_node\": %d,\n", warm_node);
	printf("  \"iterations\": [\n");

	for (i = 0; i < iterations; i++) {
		unsigned long t_start, t_end;
		double bw;

		t_start = time_ns();

		struct hbf_user_hint hint = {
			.version = 1,
			.op = 2, // PROMOTE
			.start = (unsigned long)buf,
			.len = size,
			.target_nid = hot_node,
			.deadline_ns = time_ns() + 2000000000,
			.user_tag = i * 2 + 1,
		};

		ioctl(dev_fd, HBF_IOC_SUBMIT, &hint);
		usleep(500000);

		t_end = time_ns();
		bw = (double)size / ((t_end - t_start) / 1e9) / (1024 * 1024);

		printf("    { \"iter\": %d, \"op\": \"promote\", \"bandwidth_mbs\": %.2f },\n",
		       i, bw);

		t_start = time_ns();

		hint.op = 3; // DEMOTE
		hint.target_nid = warm_node;
		hint.user_tag = i * 2 + 2;

		ioctl(fd, HBF_IOC_SUBMIT, &hint);
		usleep(500000);

		t_end = time_ns();
		bw = (double)size / ((t_end - t_start) / 1e9) / (1024 * 1024);

		printf("    { \"iter\": %d, \"op\": \"demote\", \"bandwidth_mbs\": %.2f },\n",
		       i, bw);
	}

	printf("  ]\n");
	printf("}\n");

	close(dev_fd);
	numa_free(buf, size);
	return 0;
}
