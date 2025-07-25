savedcmd_/home/prasanthi/chardevice/char.mod := printf '%s\n'   char.o | awk '!x[$$0]++ { print("/home/prasanthi/chardevice/"$$0) }' > /home/prasanthi/chardevice/char.mod
