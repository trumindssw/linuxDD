savedcmd_spinlock_bh.mod := printf '%s\n'   spinlock_bh.o | awk '!x[$$0]++ { print("./"$$0) }' > spinlock_bh.mod
