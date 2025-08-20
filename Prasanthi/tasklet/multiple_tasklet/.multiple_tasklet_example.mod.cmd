savedcmd_multiple_tasklet_example.mod := printf '%s\n'   multiple_tasklet_example.o | awk '!x[$$0]++ { print("./"$$0) }' > multiple_tasklet_example.mod
