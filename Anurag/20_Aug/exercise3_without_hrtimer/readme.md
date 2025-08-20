Exercise 3 Without HR timers
(Ioctl + Tasklets + Spinlocks + Writing into a File)

1. Setting Up the timer initially to 5s.

2. Firing the enable timer -
    a. Enabletimer = true
    b. Scheduling the tasklet

3. Tasklet Callback Function -
    a. Acquires Spinlock
    b. Write current date/time to file.
    c. Releases the lock