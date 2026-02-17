# XDP Packet Anomaly Detection Firewall

*(e1000e – Generic XDP Mode)*

---

## Objective

Implement a configurable packet-filtering firewall using **XDP (eBPF)** in the Linux networking receive path to detect and block malformed or suspicious packets.

The program runs on an Intel NIC using the **e1000e driver**, which supports only **XDP Generic Mode**.

The firewall performs early packet inspection and drops malicious packets before they reach the Linux TCP/IP stack.

---

## Features

The XDP program detects and blocks the following anomalies:

| Rule ID | Detection                              |
| ------- | -------------------------------------- |
| 0       | Invalid Ethertype                      |
| 1       | Invalid IPv4 Header Length             |
| 2       | UDP Length Mismatch                    |
| 3       | UDP Zero Port                          |
| 4       | TCP SYN + FIN flags together           |
| 5       | SYN Flood (rate-limited per source IP) |

Additional capability:

Runtime enabling/disabling of rules **without detaching or recompiling XDP**.

---

## Project Files

### `xdp_filter.c`

Kernel eBPF/XDP program.

Implements:

* Ethernet, IPv4, TCP, UDP parsing
* Anomaly detection rules
* Per-CPU drop counters
* SYN flood protection using time window hash table
* Runtime configurable rules via BPF map (`rule_config`)

---

### `Makefile`

Compiles the XDP program using clang BPF target.

Build:

```
make
```

---

### `malformed_test.py`

Scapy based packet generator.

Sends:

* ICMP packets (valid traffic)
* Invalid IP header length
* SYN + FIN packets
* UDP zero port packets
* UDP length mismatch packets
* Invalid ethertype frames
* SYN flood attack scenarios

Used to validate firewall behavior.

---

### `rule_control.py`

Interactive firewall controller (userspace CLI).

Allows:

* Enable/disable individual rules
* Enable all rules
* Disable all rules
* Load default policy
* View active rule status

This program modifies BPF maps using `bpftool` → updates kernel behavior in real time.

No XDP reload required.

---

## System Setup

### Receiver Machine (Linux – Firewall Host)

Network:
Direct Ethernet connection

Driver:

```
e1000e (Generic XDP mode)
```

Tools:

* clang
* libbpf-dev
* bpftool
* tcpdump
* python3

Assign IP:

```
sudo ip addr flush dev enp0s31f6
sudo ip addr add 169.254.154.200/16 dev enp0s31f6
sudo ip link set enp0s31f6 up
```

Attach XDP:

```
sudo ip link set dev enp0s31f6 xdp obj xdp_filter.o sec xdp
```

Check maps:

```
sudo bpftool map show
sudo bpftool map dump name dropcnt
```

---

### Sender Machine (Linux – Traffic Generator)

Requirements:

```
sudo apt install python3-scapy
```

Run packet generator:

```
sudo python3 malformed_test.py
```

---

## Firewall Runtime Control

Start rule controller:

```
sudo python3 rule_control.py
```

Example menu:

```
0 - Invalid Ethertype [Enabled]
1 - Invalid IP Header Length [Enabled]
2 - UDP Length Mismatch [Enabled]
3 - UDP Zero Port [Enabled]
4 - TCP SYN+FIN [Enabled]
5 - SYN Flood [Enabled]

e - Enable all rules
d - Disable all rules
l - Load default policy
q - Quit
```

Changes apply instantly to kernel XDP program.

---

## Verification

Monitor drop counters live:

```
watch -n1 sudo bpftool map dump name dropcnt
```

Observe:

* Valid packets → PASS
* Malformed packets → DROP
* SYN flood → dropped after threshold exceeded

Packets are filtered **before reaching TCP/IP stack**.

---

## Important Observations

* e1000e supports only XDP generic mode
* XDP runs after driver RX but before networking stack
* Drop counters stored in per-CPU BPF maps
* Driver RX drop statistics do NOT increase for XDP drops
* Firewall rules can be changed dynamically without reloading program

---

## Packet Flow

```
Sender → NIC → Driver → XDP → (DROP / PASS) → Kernel Stack
```

If dropped:
Packet never reaches socket layer.

---

## Outcome

A runtime-configurable Layer-2 firewall built using eBPF/XDP that:

* Detects malformed packets
* Mitigates SYN flood attacks
* Operates at early receive path
* Allows live rule configuration
* Requires no service restart

---