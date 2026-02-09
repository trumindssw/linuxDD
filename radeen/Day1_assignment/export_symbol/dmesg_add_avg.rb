radeen@radeen-HP-348-G7:~/work_station/qualcomm/rb3g2/my_driver$ sudo insmod export_symbol_add.ko
radeen@radeen-HP-348-G7:~/work_station/qualcomm/rb3g2/my_driver$ sudo insmod export_symbol_avg.ko
radeen@radeen-HP-348-G7:~/work_station/qualcomm/rb3g2/my_driver$ sudo dmesg | tail -n 10
[ 1743.660523] helloworld: loading out-of-tree module taints kernel.
[ 1743.660527] helloworld: module verification failed: signature and/or required key missing - tainting kernel
[ 1743.661405] I am in init module...
[ 1828.391564] I am in cleanup module...
[ 2496.018481] Add Module Loaded
[ 2496.018486] a = 7, b = 5, sum = 12
[ 2906.003737] workqueue: delayed_fput hogged CPU for >10000us 16 times, consider switching to WQ_UNBOUND
[ 3196.092449] Add Module Loaded
[ 3204.598633] Avg Module Loaded
[ 3204.598637] a=10, b=20, sum=30, avg=15