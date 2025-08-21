Exercise 3 - 
(Ioctl + Workqueues + Mutexes + Writing into a File + HR timers)

1. Setting Up the timer initially to 5s.

2. Firing the enable timer every 10s-
    a. Enabletimer = true
    b. Scheduling the workqueue

3. Workqueue Callback Function -
    a. Acquires Mutex
    b. Write current date/time to file.
    c. Hold for 5s using HR timers
    d. Releases the mutex

