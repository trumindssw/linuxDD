#include "vmlinux.h"
#include "bpf/bpf_helpers.h"
#include "bpf/bpf_endian.h"

#define ETH_P_IPV6 0x86dd
#define ETH_P_IPV4 0x0800
#define ETH_P_VLAN 0x8100


#define HOST_LEN 16
#define MAX_ENTRIES 4096

struct lpm_key {
	__u32 dest_ip;
};

struct {
    __uint(type, BPF_MAP_TYPE_LPM_TRIE);
    __uint(max_entries, MAX_ENTRIES);
    __uint(map_flags, BPF_F_NO_PREALLOC);
    __type(key, struct lpm_key);
    __type(value, __u8);
} filter_map SEC(".maps");

struct flow_key {
    __u32 src_ip;
    __u32 dest_ip;
    __u16 src_port;
    __u16 dest_port;
    __u8  proto;
}__attribute__((packed));

struct {
    __uint(type, BPF_MAP_TYPE_LRU_HASH);
    __uint(max_entries, 4096);
    __uint(map_flags, BPF_F_NO_COMMON_LRU);
    __type(key, struct flow_key);
    __type(value, __u64);
} flow_map SEC(".maps");

struct meta_mark {
    __u32 mark;
} __attribute__((alligned(4)));


static __u8 xdp_dpi_packet_filter( struct iphdr *ip) {
	struct lpm_key blk_key;
	__u8 *value;

	blk_key.dest_ip = bpf_ntohl(ip->addrs.daddr);
	value = bpf_map_lookup_elem(&filter_map, &blk_key);
	if (value != NULL)
		return 1;

	return 0;
}

static enum xdp_action xdp_dpi_udp_flow( struct iphdr *ip, struct udphdr *tcp, struct meta_mark *mark) {
	return XDP_PASS;
}

static enum xdp_action xdp_dpi_tcp_flow( struct iphdr *ip, struct tcphdr *tcp, struct meta_mark *mark) {
	struct flow_key flow_k = {0};
	__u64 *flow_val = NULL;
	__u16 payload = 0;
	__u16 tcp_h_len = tcp->doff * 4;

	flow_k.src_ip = bpf_ntohl(ip->addrs.saddr);
	flow_k.dest_ip = bpf_ntohl(ip->addrs.daddr);
	flow_k.proto = ip->protocol;
	flow_k.src_port = bpf_ntohs(tcp->source);
	flow_k.dest_port = bpf_ntohs(tcp->dest);

	if (tcp->doff < 5)
		return XDP_DROP;
	if ((tcp->fin & 1) || (tcp->rst & 1)) {
		bpf_map_delete_elem(&flow_map, &flow_k);
		return XDP_PASS;
	}

	if (tcp->syn & 1) {
		if (bpf_map_lookup_elem(&flow_map, &flow_k) != NULL)
			return XDP_PASS;
	} else {
		payload = ip->tot_len - (sizeof(struct iphdr) + tcp_h_len);
		if (payload == 0) 
			return XDP_PASS;
		flow_val = (__u64 *)bpf_map_lookup_elem(&flow_map, &flow_k);
		if (flow_val == NULL)
			return XDP_PASS;
	}
	*flow_val = bpf_ktime_get_boot_ns();
	bpf_map_update_elem(&flow_map, &flow_k, flow_val, BPF_ANY);
	//Mark this packet for netfilter queue 0x10
	mark->mark = 0x10;

	return XDP_PASS;
}

SEC("xdp_dpi")
int xdp_dpi_prog(struct xdp_md *ctx)
{
	void *data_end = NULL;
	void *data = NULL;
	void *data_meta = NULL;
	struct ethhdr *eth = NULL;
	__u16 eth_proto;
	struct iphdr *ip = NULL;
	struct tcphdr *tcp = NULL;
	struct udphdr *udp = NULL;
	struct meta_mark *mark = NULL;
	enum xdp_action action = XDP_PASS;

	if (bpf_xdp_adjust_meta(ctx, -(long)sizeof(struct meta_mark)) < 0)
		return XDP_ABORTED;

	data = (void *)(long)ctx->data;
	data_meta = (void *)(long)ctx->data_meta;
	if (data_meta + sizeof(struct meta_mark) > data)
		return XDP_ABORTED;
	mark = data_meta;

	data_end = (void *)(long)ctx->data_end;

	eth = data;
	/* Bounds check BEFORE dereference */
	if (data + sizeof(struct ethhdr) + 52  > data_end)
		return XDP_PASS;

	eth_proto = bpf_ntohs(eth->h_proto);
	if (eth_proto == ETH_P_IPV6)
	{
		bpf_printk("no action on packet if proto is  0x86dd\n");
		return XDP_PASS;
	}
	if (eth_proto == ETH_P_VLAN) {
		data = data + sizeof(struct ethhdr) + 4;
	}
	if (eth_proto == ETH_P_IPV4) {
		data = data + sizeof(struct ethhdr);
	}
	else
		return XDP_PASS;

	ip = (struct iphdr *)data;
	__u16 ip_len = bpf_ntohs(ip->tot_len);

	if ((void *)ip + (ip_len - 1) > data_end)
		return XDP_PASS;

	if (xdp_dpi_packet_filter(ip) == 1)
		return XDP_DROP;

	if (ip->protocol != 0x06 && ip->protocol != 0x14)
		return XDP_PASS;
	//fragmented packet handling to be done later

	
	if (ip->protocol == 0x06) {
		tcp = (struct tcphdr *)(ip + 1);
		if (bpf_ntohs(tcp->dest) != 443)
			return XDP_PASS;
		action = xdp_dpi_tcp_flow(ip, tcp, mark);
		return action;
	}
	if (ip->protocol == 0x14) {
		udp = (struct udphdr *)(ip + 1);
		if (bpf_ntohs(udp->dest) != 443)
			return XDP_PASS;
		action = xdp_dpi_udp_flow(ip, udp, mark);
		return action;
	}
	return XDP_PASS;

}
char _license[] SEC("license") = "GPL";
