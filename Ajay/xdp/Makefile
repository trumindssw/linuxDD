BPF_CLANG=clang
BPF_CFLAGS=-O2 -g -Wall -target bpf -I/usr/include/x86_64-linux-gnu

all:
	$(BPF_CLANG) $(BPF_CFLAGS) -c xdp_filter.c -o xdp_filter.o

clean:
	rm -f *.o
