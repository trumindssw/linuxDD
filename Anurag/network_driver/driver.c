#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/netdevice.h>
#include <linux/etherdevice.h>

MODULE_LICENSE("GPL");

static struct net_device *dummy_dev;

static int dummy_open(struct net_device *dev) {
    
    printk(KERN_INFO "Device is opened now\n");
    // making queue , which will tell that data is ready to transmit
    netif_start_queue(dev);

    return 0;
}

static int dummy_stop(struct net_device *dev) {
    
    printk(KERN_INFO "Device closed\n");
    // stoping the packets to transmit
    netif_stop_queue(dev);

    return 0;
}

static netdev_tx_t dummy_start_xmit(struct sk_buff *skb, struct net_device *dev) {
    
    printk("I'm in Dummy xmit function");
    // freeing the skb buffer    
    kfree_skb(skb);
    // Ok tells -> success , packet is transmitted successfully
    return NETDEV_TX_OK;

}

static const struct net_device_ops dummy_netdev_ops = {
    // pr_info("i am under net_device operations");
    .ndo_open = dummy_open,
    .ndo_stop = dummy_stop,
    .ndo_start_xmit = dummy_start_xmit,
};

static void dummy_setup(struct net_device *dev) {

    pr_info("i am under setup function");
    // setting the configuration to ethernet
    // mac and type to ethernet
    ether_setup(dev);
    // setting ops to net_device operations
    dev->netdev_ops = &dummy_netdev_ops;
}

static int __init dummy_init(void) {

    int result;
    pr_info("started init function");
    // creating dummy device
    dummy_dev = alloc_netdev(0, "dummy%d", NET_NAME_UNKNOWN, dummy_setup);
    if (!dummy_dev)
        return -ENOMEM;

    pr_info("in between init function");

    // once allocated we have to register it
    result = register_netdev(dummy_dev);
    if (result < 0) {
        printk(KERN_ERR "dummy: failed to register device\n");
        free_netdev(dummy_dev);
        return result;
    }

    printk(KERN_INFO "Device allocated and registered successfully");

    return 0;
}

static void __exit dummy_exit(void) {
    // freeing the allocated stuff
    unregister_netdev(dummy_dev);
    free_netdev(dummy_dev);
    printk(KERN_INFO "Device unloaded\n");
}

module_init(dummy_init);
module_exit(dummy_exit);
