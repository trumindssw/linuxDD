Instead of printing logs, sometimes you want to inspect internal state of your module on demand. For this, you can expose custom virtual files in /proc.

Reading these files triggers functions in your driver → great for debugging, monitoring, or inspecting kernel data.

method:
after insmod,
check cat /proc/myinfo


Creates /proc/myinfo on module load.
Writes a custom message on read /proc/myinfo.
After rmmod the file is myinfo is also deleted from the /proc.