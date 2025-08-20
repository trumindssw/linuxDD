#include <linux/module.h>
#include <linux/init.h>
#include <linux/i2c.h>


//This function called by the kernel when a matching i2c device found
static int dummy_probe(struct i2c_client *client)
{
    pr_info("Dummy I2C device probed: %s at address 0x%02x\n",client->name, client->addr);
    return 0;
}

//This function is called when device is removed
static void dummy_remove(struct i2c_client *client)
{
    pr_info("Dummy I2C device removed: %s\n", client->name);
}

//devices that can be interacted with this driver
static const struct i2c_device_id dummy_id[] =
{
    { "dummy_i2c", 0 },
    { }
};

MODULE_DEVICE_TABLE(i2c, dummy_id);

//i2c_driver structure
static struct i2c_driver dummy_driver = {
    .driver =
    {
        .name   = "dummy_i2c",
    },
    .probe      = dummy_probe,
    .remove     = dummy_remove,
    .id_table   = dummy_id,
};

module_i2c_driver(dummy_driver);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("SUNITHA");
MODULE_DESCRIPTION("Dummy I2C driver");

