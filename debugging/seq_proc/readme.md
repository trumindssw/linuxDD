seq_file interface, is a better and cleaner way to create /proc entries—especially when your output is larger than one page or generated iteratively (e.g., lists, tables).

Why seq_file?
Handles pagination and large outputs.

Ensures consistent output (no partial reads).

Designed for iterative structures (like arrays, lists, etc).

same steps

check the output in /proc/seqdemo