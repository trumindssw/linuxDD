from scapy.all import *
import time

TARGET_IP = "169.254.154.200"   # Ethernet Interface's IP of XDP running linux machine
DEST_MAC = "b4:45:06:04:6c:a4"  # Linux NIC MAC address (running XDP)
INTERFACE = "enp3s0"         # Ethernet interface name of sender


# ---------------- Basic Rule Tests ----------------

# ICMP test (should PASS)
print("[*] Sending ICMP packets...")
pkt0 = Ether(dst=DEST_MAC)/IP(dst=TARGET_IP)/ICMP()
sendp(pkt0, iface=INTERFACE, count=10, verbose=False)

# Invalid IP header length (should DROP)
print("[*] Sending Invalid IP Header Length packets...")
pkt_iphl = Ether(dst=DEST_MAC)/IP(dst=TARGET_IP)/TCP(dport=80)

# Force invalid IHL (<5)
pkt_iphl[IP].ihl = 4
del pkt_iphl[IP].len
del pkt_iphl[IP].chksum

sendp(pkt_iphl, iface=INTERFACE, count=10, verbose=False)

# SYN+FIN test (should DROP)
print("[*] Sending SYN+FIN packets...")
pkt1 = Ether(dst=DEST_MAC)/IP(dst=TARGET_IP)/TCP(sport=1234, dport=80, flags="SF")
sendp(pkt1, iface=INTERFACE, count=10, verbose=False)

# UDP zero source port (should DROP)
print("[*] Sending Zero Port packets...")
pkt2 = Ether(dst=DEST_MAC)/IP(dst=TARGET_IP)/UDP(sport=0, dport=80)
sendp(pkt2, iface=INTERFACE, count=10, verbose=False)

# UDP length mismatch (should DROP)
print("[*] Sending UDP length mismatch packets...")
pkt3 = Ether(dst=DEST_MAC)/IP(dst=TARGET_IP)/UDP(sport=1234, dport=80)/Raw(b"AAAA")
pkt3[UDP].len = 5
del pkt3[UDP].chksum
del pkt3[IP].len
del pkt3[IP].chksum
sendp(pkt3, iface=INTERFACE, count=10, verbose=False)

# Invalid Ethertype (should DROP)
print("[*] Sending Invalid Ethertype packets...")
pkt4 = Ether(dst=DEST_MAC, type=0x1234)/IP(dst=TARGET_IP)/TCP(dport=80)
sendp(pkt4, iface=INTERFACE, count=10, verbose=False)


# ---------------- SYN Flood Scenarios ----------------

pkt_syn = Ether(dst=DEST_MAC)/IP(dst=TARGET_IP)/TCP(
    sport=RandShort(),
    dport=80,
    flags="S"
)

# Scenario 1: Fast burst (should trigger drop after 100)
print("[*] Sending SYN packets...")
print("[*] Scenario 1: Fast burst (300 SYN instantly)")
sendp(pkt_syn, iface=INTERFACE, count=300, inter=0, verbose=False)

time.sleep(2)  # wait 2 seconds so window resets


# Scenario 2: Slow rate (10 per second - should NOT trigger drop)
print("[*] Scenario 2: Slow rate (10 SYN/sec)")
sendp(pkt_syn, iface=INTERFACE, count=30, inter=0.1, verbose=False)

time.sleep(2)  # wait again to reset window


# Scenario 3: Burst → wait → burst again
print("[*] Scenario 3: 120 SYN fast (should drop after 100)")
sendp(pkt_syn, iface=INTERFACE, count=120, inter=0, verbose=False)

time.sleep(2)  # allow reset

print("[*] Scenario 3 continued: Another 120 SYN fast (should drop again)")
sendp(pkt_syn, iface=INTERFACE, count=120, inter=0, verbose=False)


print("[+] All SYN flood scenarios completed.")
