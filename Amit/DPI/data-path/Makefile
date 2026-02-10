XDP_INTERFACE = wlp3s0

TOP = $(abspath $(dir $(firstword $(MAKEFILE_LIST))))

TARGET = bpf
WORKSPACE = $(TOP)/..
SRC_PATH = $(WORKSPACE)/data-path
INC_BASE = $(WORKSPACE)

PROG = xdp_dpi.o

CLANG = clang

CFLAGS = -O2 -g -Wall -target $(TARGET) -I$(INC_BASE)/include

all: $(PROG)

$(PROG): *.c
	$(CLANG) $(CFLAGS) -c $< -o $@

install:
	ip link set dev $(XDP_INTERFACE) xdp obj xdp_dpi.o sec xdp_dpi

uninstall:
	ip link set dev $(XDP_INTERFACE) xdp off

clean: 
	rm -f $(TOP)/*.o
