#ifndef MYDRV_IOCTL_H
#define MYDRV_IOCTL_H

#include <linux/ioctl.h>

#define MY_MAGIC 'k'

#define MYDRV_ENABLE_TIMER   _IO(MY_MAGIC, 1)
#define MYDRV_SET_TIMER_VAL  _IOW(MY_MAGIC, 2, int)
#define MYDRV_STOP_TIMER     _IO(MY_MAGIC, 3)

#endif
