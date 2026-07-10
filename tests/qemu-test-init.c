// SPDX-License-Identifier: GPL-2.0
// QEMU boot-time test for HBF Control Plane /dev/hbfctl
// Compile: x86_64-linux-gnu-gcc -static -I<kernel>/include/uapi -I<kernel>/include $< -o $@
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/mount.h>
#include <sys/syscall.h>
#include <linux/ioctl.h>
#include <linux/types.h>
#include <linux/hbf.h>

int main(void) {
    printf("HBF Control Plane v0.2 QEMU Test\n");
    printf("=================================\n");

    if (mount("devtmpfs", "/dev", "devtmpfs", 0, NULL) == 0)
        printf("Mount devtmpfs: OK\n");
    else {
        perror("mount devtmpfs");
        return 1;
    }

    int fd = open("/dev/hbfctl", O_RDWR);
    if (fd < 0) {
        perror("open /dev/hbfctl");
        return 1;
    }
    printf("OK: /dev/hbfctl opened\n");

    struct hbf_caps caps;
    memset(&caps, 0, sizeof(caps));
    if (ioctl(fd, HBF_IOC_CAPS, &caps) < 0) {
        perror("HBF_IOC_CAPS");
        return 1;
    }
    printf("OK: HBF_IOC_CAPS returns:\n");
    printf("    max_request_bytes=%llu\n", caps.max_request_bytes);
    printf("    max_inflight=%u\n", caps.max_inflight);
    printf("    supported_ops=0x%x\n", caps.supported_ops);
    printf("    max_target_nodes=%u\n", caps.max_target_nodes);

    printf("ALL TESTS PASSED\n");
    close(fd);
    return 0;
}
