#include <errno.h>
#include <fcntl.h>
#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <unistd.h>

#define HBF_UAPI_VERSION 1U

#define HBF_HINT_PREFETCH 1U
#define HBF_HINT_PROMOTE  2U
#define HBF_HINT_DEMOTE   3U
#define HBF_HINT_PIN      4U
#define HBF_HINT_RELEASE  5U

/*
 * Experimental ioctl number for the RFC repository.
 * The prototype patch currently uses the same command number under the name
 * HBF_IOC_SUBMIT_HINT.
 */
#define HBF_IOC_MAGIC 'H'

struct hbf_user_hint {
	uint32_t version;
	uint32_t op;
	uint64_t flags;
	uint64_t va;
	uint64_t len;
	uint64_t object_id;
	uint32_t object_class;
	int32_t target_node;
	uint64_t deadline_ns;
	uint64_t expected_reuse_ns;
	uint64_t reserved[8];
};

#define HBF_IOC_HINT _IOW(HBF_IOC_MAGIC, 0x01, struct hbf_user_hint)

static void usage(const char *prog)
{
	fprintf(stderr,
		"Usage:\n"
		"  %s --prefetch 0xADDR LEN\n"
		"  %s --promote  0xADDR LEN\n"
		"  %s --demote   0xADDR LEN\n"
		"  %s --pin      0xADDR LEN\n"
		"  %s --release  0xADDR LEN\n",
		prog, prog, prog, prog, prog);
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

static uint32_t parse_op(const char *arg)
{
	if (strcmp(arg, "--prefetch") == 0)
		return HBF_HINT_PREFETCH;
	if (strcmp(arg, "--promote") == 0)
		return HBF_HINT_PROMOTE;
	if (strcmp(arg, "--demote") == 0)
		return HBF_HINT_DEMOTE;
	if (strcmp(arg, "--pin") == 0)
		return HBF_HINT_PIN;
	if (strcmp(arg, "--release") == 0)
		return HBF_HINT_RELEASE;

	return 0;
}

int main(int argc, char **argv)
{
	struct hbf_user_hint hint;
	uint64_t addr;
	uint64_t len;
	int fd;

	if (argc != 4) {
		usage(argv[0]);
		return 2;
	}

	if (parse_u64(argv[2], &addr) != 0 || parse_u64(argv[3], &len) != 0) {
		fprintf(stderr, "Invalid address or length.\n");
		return 2;
	}

	memset(&hint, 0, sizeof(hint));
	hint.version = HBF_UAPI_VERSION;
	hint.op = parse_op(argv[1]);
	hint.va = addr;
	hint.len = len;
	hint.target_node = -1;

	if (hint.op == 0) {
		usage(argv[0]);
		return 2;
	}

	fd = open("/dev/hbfctl", O_RDWR);
	if (fd < 0) {
		if (errno == ENOENT) {
			fprintf(stderr,
				"/dev/hbfctl not found. This is expected unless the RFC kernel module/patch is loaded.\n");
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

	printf("submitted experimental HBF hint: op=%" PRIu32 " addr=0x%" PRIx64 " len=%" PRIu64 "\n",
	       hint.op, hint.va, hint.len);

	close(fd);
	return 0;
}
