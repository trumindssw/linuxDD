radeen@radeen-HP-348-G7:~/work_station/qualcomm/rb3g2/my_driver$ sudo insmod module_param.ko a=7 b=5
radeen@radeen-HP-348-G7:~/work_station/qualcomm/rb3g2/my_driver$ sudo dmesg | tail -n 10
[ 1711.940172] atkbd serio0: Unknown key pressed (translated set 2, code 0xab on isa0060/serio0).
[ 1711.940195] atkbd serio0: Use 'setkeycodes e02b <keycode>' to make it known.
[ 1711.951231] atkbd serio0: Unknown key released (translated set 2, code 0xab on isa0060/serio0).
[ 1711.951251] atkbd serio0: Use 'setkeycodes e02b <keycode>' to make it known.
[ 1743.660523] helloworld: loading out-of-tree module taints kernel.
[ 1743.660527] helloworld: module verification failed: signature and/or required key missing - tainting kernel
[ 1743.661405] I am in init module...
[ 1828.391564] I am in cleanup module...
[ 2496.018481] Add Module Loaded
[ 2496.018486] a = 7, b = 5, sum = 12