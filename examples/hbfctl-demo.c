#include <errno.h>
#include <fcntl.h>
#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <unistd.h>

#define HBF_HINT_PREFETCH 1U
#define HBF_HINT_PROMOTE  2U
#define HBF_HINT_DEMOTE   3U
#define HBF_HINT_PIN      4U
#define HBF_HINT_RELEASE  5U

struct hbf_hint {
	void *addr;
	size_t len;
	uint32_t op;
	uint32_t flags;
	uint64_t deadline_ns;
	uint64_t user_tag;
};

#define HBF_IOC_MAGIC 'H'
#define HBF_IOC_HINT _IOW(HBF_IOC_MAGIC, 0x01, struct hbf_hint)

static void usage(const char *prog)
{
	fprintf(stderr,
		"Usage:\n"
		"  %s --prefetch 0xADDR LEN\n"
		"  %s --promote  0xADDR LEN\n"
		"  %s --demote   0xADDR LEN\n",
		prog, prog, prog);
}

static int parse_u64(const char *arg, uint64_t *value)
{
	char *end = NULL;
	unsigned long long parsed;

	errno = 0;
	parsed = strtoull(arg, &end, 0);
	if (errno || !end || *end != '\0')
		return -1;

	*value = (uint64_t)parsed;
	return 0;
}

static int parse_len(const char *arg, size_t *len)
{
	uint64_t tmp;

	if (parse_u64(arg, &tmp) != 0)
		return -1;

	*len = (size_t)tmp;
	return 0;
}

static uint32_t parse_op(const char *arg)
{
	if (strcmp(arg, "--prefetch") == 0)
		return HBF_HINT_PREFETCH;
	if (strcmp(arg, "--promote") == 0)
		return HBF_HINT_PROMOTE;
	if (strcmp(arg, "--demote") == 0)
		return HBF_HINT_DEMOTE;

	return 0;
}

int main(int argc, char **argv)
{
	struct hbf_hint hint;
	uint64_t addr_u64;
	size_t len;
	uint32_t op;
	int fd;

	if (argc != 4) {
		usage(argv[0]);
		return 2;
	}

	op = parse_op(argv[1]);
	if (!op) {
		usage(argv[0]);
		return 2;
	}

	if (parse_u64(argv[2], &addr_u64) != 0 || parse_len(argv[3], &len) != 0) {
		fprintf(stderr, "Invalid address or length.\n");
		return 2;
	}

	memset(&hint, 0, sizeof(hint));
	hint.addr = (void *)(uintptr_t)addr_u64;
	hint.len = len;
	hint.op = op;
	hint.flags = 0;
	hint.deadline_ns = 0;
	hint.user_tag = 0;

	fd = open("/dev/hbfctl", O_RDWR);
	if (fd < 0) {
		if (errno == ENOENT) {
			fprintf(stderr,
				"/dev/hbfctl is not present.\n"
				"This repository is an RFC/mock example for a proposed HBF control plane,\n"
				"so no upstream kernel device is expected to exist yet.\n");
			return 0;
		}

		perror("open(/dev/hbfctl)");
		return 1;
	}

	if (ioctl(fd, HBF_IOC_HINT, &hint) < 0) {
		perror("ioctl(HBF_IOC_HINT)");
		close(fd);
		return 1;
	}

	printf("Submitted HBF hint: op=%" PRIu32 " addr=%p len=%zu\n",
	       hint.op, hint.addr, hint.len);

	close(fd);
	return 0;
}
