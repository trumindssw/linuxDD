savedcmd_tasklets.mod := printf '%s\n'   tasklets.o | awk '!x[$$0]++ { print("./"$$0) }' > tasklets.mod
