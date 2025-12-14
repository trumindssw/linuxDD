savedcmd_spinlocks.mod := printf '%s\n'   spinlocks.o | awk '!x[$$0]++ { print("./"$$0) }' > spinlocks.mod
