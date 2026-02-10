#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>

#include <bpf/libbpf.h>
#include <bpf/bpf.h>

#define MAP_NAME "xdp_cfg"

#define HOST_LEN 64

struct lpm_key {
	__u32 prefixlen;
	char domain[HOST_LEN];
};

#define MAP_PATH "/sys/fs/bpf/domain_map"

static void reverse_domain(char *dst, const char *src)
{
    int len = strlen(src);
    for (int i = 0; i < len; i++)
        dst[i] = src[len - 1 - i];
}

int main(int argc, char **argv)
{
    int map_fd;
    struct lpm_key key;
    __u8 value;

    if (argc != 3) {
        fprintf(stderr, "Usage: %s domain-name  1/0\n", argv[0]);
        return 1;
    }

    value = atoi(argv[2]);

    /* Option A: open pinned map */
    map_fd = bpf_obj_get(MAP_PATH);
    if (map_fd < 0) {
        perror("bpf_obj_get");
        return 1;
    }

    reverse_domain(key.domain, argv[1]);
    key.prefixlen = strlen(argv[1]) * 8;

    /* Update map */
    if (value == 1) {
    	if (bpf_map_update_elem(map_fd, &key, &value, BPF_ANY) < 0) {
        	perror("bpf_map_update_elem");
        	return 1;
    	}
    }
    else if (value == 0) {
	if (bpf_map_delete_elem(map_fd, &key) < 0) {
		perror("bpf_map_delete_elem");
		return 1;
	    }
    }
    else {
    	fprintf(stderr, "Usage: %s domain-name  1/0\n", argv[0]);
        return 1;
    }

    return 0;
}

