Here, we observe kernel variables live through /proc by exposing kernel state to user space.

# example process:

Exposes a kernel variable (e.g., int counter = 0) via /proc/counter

Allows user to read its value

Update it with echo N > /proc/counter
