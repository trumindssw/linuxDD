savedcmd_spinlock_normal.mod := printf '%s\n'   spinlock_normal.o | awk '!x[$$0]++ { print("./"$$0) }' > spinlock_normal.mod
