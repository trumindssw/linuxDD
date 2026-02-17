#include <linux/bpf.h>
#include <bpf/bpf_helpers.h>
#include <bpf/bpf_endian.h>
#include <linux/if_ether.h>
#include <linux/ip.h>
#include <linux/udp.h>
#include <linux/tcp.h>
#include <linux/in.h>

#define MAX_SYN_THRESHOLD 100
#define WINDOW_NS 1000000000ULL

/* ---------------- Drop Reason Enum ---------------- */

enum drop_reasons {
    DROP_INVALID_ETH = 0,
    DROP_INVALID_IPHL,
    DROP_UDP_LEN_MISMATCH,
    DROP_UDP_ZERO_PORT,
    DROP_SYN_FIN,
    DROP_SYN_FLOOD,
    DROP_MAX
};

/* ---------------- Drop Counter Map ---------------- */

struct {
    __uint(type, BPF_MAP_TYPE_PERCPU_ARRAY);
    __uint(max_entries, DROP_MAX);
    __type(key, __u32);
    __type(value, __u64);
} dropcnt SEC(".maps");

/* ---------------- Rule Config Map ---------------- */

struct {
    __uint(type, BPF_MAP_TYPE_ARRAY);
    __uint(max_entries, DROP_MAX);
    __type(key, __u32);
    __type(value, __u32);
} rule_config SEC(".maps");

/* ---------------- SYN Tracking ---------------- */

struct syn_info {
    __u64 window_start;
    __u32 count;
};

struct {
    __uint(type, BPF_MAP_TYPE_HASH);
    __uint(max_entries, 1024);
    __type(key, __u32);
    __type(value, struct syn_info);
} syn_map SEC(".maps");

/* ---------------- Helper: Check Rule Enabled ---------------- */

static __always_inline int rule_enabled(__u32 key)
{
    __u32 *enabled = bpf_map_lookup_elem(&rule_config, &key);
    if (enabled && *enabled)
        return 1;
    return 0;
}

/* ---------------- Helper: Increment Drop Counter ---------------- */

static __always_inline void increment_drop(__u32 key)
{
    __u64 *val = bpf_map_lookup_elem(&dropcnt, &key);
    if (val)
        (*val)++;
}

/* ---------------- XDP Program ---------------- */

SEC("xdp")
int xdp_filter(struct xdp_md *ctx)
{
    void *data_end = (void *)(long)ctx->data_end;
    void *data = (void *)(long)ctx->data;

    struct ethhdr *eth = data;
    if ((void *)(eth + 1) > data_end)
        return XDP_ABORTED;

    /* Rule: Invalid Ethertype */
    if (rule_enabled(DROP_INVALID_ETH)) {

    if (eth->h_proto != __constant_htons(ETH_P_IP) &&
        eth->h_proto != __constant_htons(ETH_P_ARP) &&
        eth->h_proto != __constant_htons(ETH_P_IPV6)) {

        increment_drop(DROP_INVALID_ETH);
        return XDP_DROP;
    }
}


    struct iphdr *ip = data + sizeof(*eth);
    if ((void *)(ip + 1) > data_end)
        return XDP_ABORTED;

    /* Rule: Invalid IP header length */
    if (rule_enabled(DROP_INVALID_IPHL)) {
        if (ip->ihl < 5) {
            increment_drop(DROP_INVALID_IPHL);
            return XDP_DROP;
        }
    }

    /* ---------------- UDP Rules ---------------- */

    if (ip->protocol == IPPROTO_UDP) {

        struct udphdr *udp = (void *)ip + ip->ihl * 4;
        if ((void *)(udp + 1) > data_end)
            return XDP_ABORTED;

        if (rule_enabled(DROP_UDP_LEN_MISMATCH)) {
            if (bpf_ntohs(udp->len) !=
                (bpf_ntohs(ip->tot_len) - ip->ihl * 4)) {
                increment_drop(DROP_UDP_LEN_MISMATCH);
                return XDP_DROP;
            }
        }

        if (rule_enabled(DROP_UDP_ZERO_PORT)) {
            if (udp->source == 0 || udp->dest == 0) {
                increment_drop(DROP_UDP_ZERO_PORT);
                return XDP_DROP;
            }
        }
    }

    /* ---------------- TCP Rules ---------------- */

    if (ip->protocol == IPPROTO_TCP) {

        struct tcphdr *tcp = (void *)ip + ip->ihl * 4;
        if ((void *)(tcp + 1) > data_end)
            return XDP_ABORTED;

        if (rule_enabled(DROP_SYN_FIN)) {
            if (tcp->syn && tcp->fin) {
                increment_drop(DROP_SYN_FIN);
                return XDP_DROP;
            }
        }

        if (rule_enabled(DROP_SYN_FLOOD)) {

            if (tcp->syn && !tcp->ack) {

                __u32 src_ip = ip->saddr;
                __u64 now = bpf_ktime_get_ns();

                struct syn_info *info =
                    bpf_map_lookup_elem(&syn_map, &src_ip);

                if (info) {

                    if (now - info->window_start > WINDOW_NS) {
                        info->window_start = now;
                        info->count = 1;
                    } else {
                        info->count++;
                    }

                    if (info->count > MAX_SYN_THRESHOLD) {
                        increment_drop(DROP_SYN_FLOOD);
                        return XDP_DROP;
                    }

                } else {

                    struct syn_info new_info = {};
                    new_info.window_start = now;
                    new_info.count = 1;

                    bpf_map_update_elem(&syn_map,
                                        &src_ip,
                                        &new_info,
                                        BPF_ANY);
                }
            }
        }
    }

    return XDP_PASS;
}

char _license[] SEC("license") = "GPL";
