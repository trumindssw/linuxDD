savedcmd_spinlock_irq.mod := printf '%s\n'   spinlock_irq.o | awk '!x[$$0]++ { print("./"$$0) }' > spinlock_irq.mod
