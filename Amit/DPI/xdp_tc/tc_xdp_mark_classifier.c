#include "vmlinux.h"
#include <bpf/bpf_helpers.h>
#include <bpf/bpf_endian.h>


#define TC_ACT_OK        0
#define TC_ACT_RECLASSIFY 1
#define TC_ACT_SHOT      2
#define TC_ACT_PIPE      3

struct meta_data {
    __u32 mark;
};

SEC("classifier")
int tc_set_mark(struct __sk_buff *skb)
{
    void *meta = (void *)(long)skb->data_meta;
    void *data = (void *)(long)skb->data;

    if (meta + sizeof(struct meta_data) > data)
        return TC_ACT_PIPE;

    struct meta_data *m = meta;

    if (m->mark) {
        skb->mark = m->mark;
    	return TC_ACT_OK;
    }

    return TC_ACT_PIPE;
}
