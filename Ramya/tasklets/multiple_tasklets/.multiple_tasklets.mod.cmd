savedcmd_multiple_tasklets.mod := printf '%s\n'   multiple_tasklets.o | awk '!x[$$0]++ { print("./"$$0) }' > multiple_tasklets.mod
