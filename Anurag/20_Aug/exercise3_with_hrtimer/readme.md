Exercise 3 With HR timers
(Ioctl + Tasklets + Spinlocks + Writing into a File + HR timers)

1. Setting Up the timer initially to 5s.

2. Firing the enable timer -
    a. Enabletimer = true
    b. Scheduling the tasklet

3. Tasklet Callback Function -
    a. Acquires Spinlock
    b. Write current date/time to file.
    c. Releases the lock

As tasklets + spinlocks runs is softirq context (atomic) we cant hold the spinlocks by using hr timers
As we are holding it so we are getting this bug in dmesg ---->

![alt text](image.png)