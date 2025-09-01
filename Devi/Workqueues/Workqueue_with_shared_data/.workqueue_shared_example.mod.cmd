savedcmd_workqueue_shared_example.mod := printf '%s\n'   workqueue_shared_example.o | awk '!x[$$0]++ { print("./"$$0) }' > workqueue_shared_example.mod
