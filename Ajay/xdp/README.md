# XDP Packet Anomaly Detection (e1000e – Generic Mode)

## Task

Use **XDP (Express Data Path)** in the Ethernet driver to detect packet anomalies such as:

* Invalid Ethertype
* Invalid IP header length
* UDP length mismatch
* Zero port
* TCP SYN + FIN in same packet
* SYN Flood attack

Since the Intel **e1000e** driver does not support native XDP, the program runs in **XDP generic mode**.

---

## Files in This Project

### 1. `xdp_filter.c`

XDP program written in C.

Implements:

* Packet parsing (Ethernet, IPv4, TCP, UDP)
* Anomaly detection rules
* Drop counters using BPF maps
* SYN flood detection using hash map

---

### 2. `Makefile`

Used to compile the XDP program using clang with BPF target.

Build command:

```bash
make
```

---

### 3. `malformed_test.py`

Python script (Scapy-based) used from Windows machine to generate:

* ICMP packets
* SYN + FIN packets
* Zero port packets
* UDP length mismatch packets
* Invalid Ethertype packets
* SYN flood packets

---

## Setup Used

### Linux Machine

* Intel NIC using `e1000e` driver
* XDP attached in generic mode
* Manual IPv4 configuration (direct Ethernet cable setup)
* Tools used:

  * clang
  * libbpf-dev
  * bpftool
  * tcpdump

Attach XDP:

```bash
sudo ip link set dev enp0s31f6 xdp obj xdp_filter.o sec xdp
```

Check counters:

```bash
sudo bpftool map dump name dropcnt
```

---

### Windows Machine

* Connected via direct Ethernet cable
* Python + Scapy
* Npcap installed
* Used to generate malformed packets and SYN flood

---

## What Was Verified

* RX packets increase at driver level
* XDP drop counters increase
* Packets are dropped before reaching TCP/IP stack
* SYN flood detection works after threshold is exceeded

---

## Notes

* e1000e supports only generic XDP
* Drop happens after skb allocation
* Driver RX dropped stats do not increase for XDP_DROP