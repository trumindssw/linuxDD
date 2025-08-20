Exercise 3 With Workqueues + Mutexes
(Ioctl + Workqueues + Mutexes + Writing into a File + HR timers)

1. Setting Up the timer initially to 5s.

2. Firing the enable timer -
    a. Enabletimer = true
    b. Scheduling the tasklet

3. Tasklet Callback Function -
    a. Acquires Spinlock
    b. Write current date/time to file.
    c. Releases the lock

NO ERROR 
As workqueues + mutexes both can sleep (process context) 