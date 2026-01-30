#clang -O2 -g -Wall -I../include/  -target bpf   -D__TARGET_ARCH_x86   -c tc_xdp_mark_classifier.c   -o tc_xdp_mark_classifier.o

CLANG = clang

PROG = tc_xdp.o
TARGET = bpf
TOPDIR = $(PWD)/..
INC_PATH = $(TOPDIR)/include
ARCH = __TARGET_ARCH_x86

all:$(PROG)

$(PROG): *.c
	$(CLANG) $(CFLAGS) -I$(INC_PATH)/ -target $(TARGET) -D$(ARCH) -c $< -o $@

clean:
	rm -f *.o
