savedcmd_tasklets_irq.mod := printf '%s\n'   tasklets_irq.o | awk '!x[$$0]++ { print("./"$$0) }' > tasklets_irq.mod
