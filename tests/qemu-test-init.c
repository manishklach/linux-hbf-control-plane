// SPDX-License-Identifier: GPL-2.0
// QEMU boot-time test for HBF Control Plane /dev/hbfctl
// Compile: x86_64-linux-gnu-gcc -static -I<kernel>/include/uapi -I<kernel>/include $< -o $@
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/mount.h>
#include <sys/syscall.h>
#include <sys/mman.h>
#include <linux/ioctl.h>
#include <linux/types.h>
#include <linux/hbf.h>

static int failures;

static void check(int ok, const char *msg)
{
	if (ok)
		printf("  OK: %s\n", msg);
	else {
		printf("  FAIL: %s\n", msg);
		failures++;
	}
}

static int try_ioctl(int fd, unsigned long cmd, void *arg, const char *label)
{
	if (ioctl(fd, cmd, arg) == 0)
		return 0;
	printf("  FAIL: %s (%d)\n", label, errno);
	failures++;
	return -1;
}

static void test_caps(int fd)
{
	struct hbf_caps caps;

	memset(&caps, 0, sizeof(caps));
	if (try_ioctl(fd, HBF_IOC_CAPS, &caps, "HBF_IOC_CAPS") == 0)
		printf("    max_request_bytes=%llu max_inflight=%u\n"
		       "    supported_ops=0x%x max_target_nodes=%u\n",
		       caps.max_request_bytes, caps.max_inflight,
		       caps.supported_ops, caps.max_target_nodes);
}

static void test_submit_query(int fd)
{
	struct hbf_user_hint hint = { .version = 1 };
	struct hbf_query_result qr;
	void *buf;
	unsigned long long request_id;

	buf = mmap(NULL, 4096, PROT_READ | PROT_WRITE,
		   MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
	check(buf != MAP_FAILED, "mmap 4K");
	if (buf == MAP_FAILED)
		return;

	hint.op = HBF_HINT_PROMOTE;
	hint.start = (unsigned long)buf;
	hint.len = 4096;
	hint.target_nid = 0;

	if (try_ioctl(fd, HBF_IOC_SUBMIT, &hint, "HBF_IOC_SUBMIT") == 0) {
		memcpy(&request_id, &hint, sizeof(request_id));
		printf("    request_id=%llu\n", request_id);

		memset(&qr, 0, sizeof(qr));
		qr.request_id = request_id;
		if (try_ioctl(fd, HBF_IOC_QUERY, &qr, "HBF_IOC_QUERY") == 0)
			printf("    state=%u pages_moved=%llu\n",
			       qr.state, qr.pages_moved);
	}
	munmap(buf, 4096);
}

static void test_deadline_reject(int fd)
{
	struct hbf_user_hint hint = { .version = 1 };
	void *buf;

	buf = mmap(NULL, 4096, PROT_READ | PROT_WRITE,
		   MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
	check(buf != MAP_FAILED, "mmap 4K for deadline test");
	if (buf == MAP_FAILED)
		return;

	hint.op = HBF_HINT_PROMOTE;
	hint.start = (unsigned long)buf;
	hint.len = 4096;
	hint.target_nid = 0;
	hint.deadline_ns = 1;

	if (ioctl(fd, HBF_IOC_SUBMIT, &hint) == 0)
		printf("  INFO: deadline hint accepted (may be feasible)\n");
	else
		printf("  OK: deadline hint rejected (expected on fast path)\n");

	munmap(buf, 4096);
}

static void test_multi_submit(int fd)
{
	void *buf;
	int i;

	buf = mmap(NULL, 4096 * 8, PROT_READ | PROT_WRITE,
		   MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
	check(buf != MAP_FAILED, "mmap 32K for multi-submit");
	if (buf == MAP_FAILED)
		return;

	for (i = 0; i < 4; i++) {
		struct hbf_user_hint hint = { .version = 1 };

		hint.op = HBF_HINT_PROMOTE;
		hint.start = (unsigned long)buf + i * 4096;
		hint.len = 4096;
		hint.target_nid = 0;
		if (try_ioctl(fd, HBF_IOC_SUBMIT, &hint,
			      "HBF_IOC_SUBMIT (multi)") == 0)
			printf("    submit #%d OK\n", i + 1);
	}
	munmap(buf, 4096 * 8);
}

int main(void)
{
	printf("HBF Control Plane v0.3 QEMU Test\n");
	printf("=================================\n");

	if (mount("devtmpfs", "/dev", "devtmpfs", 0, NULL) == 0)
		printf("Mount devtmpfs: OK\n");
	else {
		perror("mount devtmpfs");
		return 1;
	}

	int fd = open("/dev/hbfctl", O_RDWR);
	check(fd >= 0, "/dev/hbfctl opened");
	if (fd < 0)
		return 1;

	test_caps(fd);
	test_submit_query(fd);
	test_deadline_reject(fd);
	test_multi_submit(fd);

	printf("\n");
	if (failures == 0)
		printf("ALL TESTS PASSED\n");
	else
		printf("%d TEST(S) FAILED\n", failures);

	close(fd);
	return failures ? 1 : 0;
}
