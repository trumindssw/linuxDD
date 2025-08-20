savedcmd_tasklet_irq_example.mod := printf '%s\n'   tasklet_irq_example.o | awk '!x[$$0]++ { print("./"$$0) }' > tasklet_irq_example.mod
