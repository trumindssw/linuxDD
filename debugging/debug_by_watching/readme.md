Here, we observe kernel variables live through /proc and /dev interfaces by exposing kernel state to user space.

# example:

we can read and write the value using the  cat /dev/watch_drv and echo <value> > /dev/watch_drv using the driver read and write funtions (file_operations).

also, cat /proc/watch_info and echo <value> > /proc/watch_info respectively using the proc read and write funtions (proc_ops).

