savedcmd_dynamic_alloc_char_driver.mod := printf '%s\n'   dynamic_alloc_char_driver.o | awk '!x[$$0]++ { print("./"$$0) }' > dynamic_alloc_char_driver.mod
