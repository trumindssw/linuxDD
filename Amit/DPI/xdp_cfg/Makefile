
PROG = xdp_cfg

CFLAGS = -O2 -Wall

LIBS = -lbpf -lelf -lz

INC_PATH = $(PWD)/../include

all: $(PROG)

$(PROG): *.c
	gcc $(CFLAGS) -I$(INC_PATH) $< -o $@ $(LIBS)

clean:
	rm -f xdp_cfg
