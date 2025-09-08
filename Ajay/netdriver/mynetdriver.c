#include <linux/module.h>
#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <linux/skbuff.h>

#define DRV_NAME "tss_simple"
#define NUM_DEVS 2   // As we have to create two NICs

static struct net_device *tss_devs[NUM_DEVS];

// Function for transmitting the packets
static netdev_tx_t tss_start_xmit(struct sk_buff *skb, struct net_device *dev)
{
    pr_info("xmit being called");   
    struct sk_buff *rx_skb;

    // update TX statistics
    dev->stats.tx_packets++;
    dev->stats.tx_bytes += skb->len;

    // clone skb for RX (so we can free the original)
    rx_skb = skb_clone(skb, GFP_ATOMIC);
    if (rx_skb) {
        // set device and protocol for RX skb
        rx_skb->dev = dev;
        rx_skb->protocol = eth_type_trans(rx_skb, dev);

        // pass to network stack
        netif_rx(rx_skb);

        // update RX statistics
        dev->stats.rx_packets++;
        dev->stats.rx_bytes += rx_skb->len;
    }

    // free the original skb
    dev_kfree_skb(skb);

    return NETDEV_TX_OK;
}

// opens the network device
static int tss_open(struct net_device *dev)
{
    netif_start_queue(dev);
    pr_info("%s: device opened\n", dev->name);
    return 0;
}

//closes the network device
static int tss_stop(struct net_device *dev)
{
    netif_stop_queue(dev);
    pr_info("%s: device closed\n", dev->name);
    return 0;
}

// net_device_ops for operations supported by the network devices we create
static const struct net_device_ops tss_netdev_ops = {
    .ndo_open       = tss_open,
    .ndo_stop       = tss_stop,
    .ndo_start_xmit = tss_start_xmit,
};

// setup function
static void tss_setup(struct net_device *dev)
{
    ether_setup(dev);                 // fill in Ethernet device defaults
    dev->netdev_ops = &tss_netdev_ops;

    // assign a fake MAC address
    unsigned char mac[ETH_ALEN] = {0x02, 0x11, 0x22, 0x33, 0x44,
                               (u8)(get_random_u32() & 0xFF)};
    eth_hw_addr_set(dev, mac);

}

//module init
static int __init tss_init(void)
{
    int i, ret;

    pr_info(DRV_NAME ": loading\n");

    for (i = 0; i < NUM_DEVS; i++) {
        // alloc and setup netdev
        tss_devs[i] = alloc_netdev(0, "tss0%d",
                                   NET_NAME_UNKNOWN, tss_setup);
        if (!tss_devs[i])
            return -ENOMEM;

        ret = register_netdev(tss_devs[i]);
        if (ret) {
            pr_err("Failed to register %s\n", tss_devs[i]->name);
            free_netdev(tss_devs[i]);
            return ret;
        }

        pr_info("%s registered\n", tss_devs[i]->name);
    }
    return 0;
}

//module exit
static void __exit tss_exit(void)
{
    int i;
    pr_info(DRV_NAME ": unloading\n");

    for (i = 0; i < NUM_DEVS; i++) {
        if (tss_devs[i]) {
            unregister_netdev(tss_devs[i]);
            free_netdev(tss_devs[i]);
            tss_devs[i] = NULL;
        }
    }
}

module_init(tss_init);
module_exit(tss_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Ajaypal Singh");
MODULE_DESCRIPTION("Simple pseudo NIC driver (loopback only)");
