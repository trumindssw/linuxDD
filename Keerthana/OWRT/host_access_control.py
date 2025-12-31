#!/usr/bin/env python3
"""
===============================================================================
Host Control Script for OpenWrt
===============================================================================

Author      : (Your Name)
Description : Unified host control utility for OpenWrt that supports:
              1. Time-based internet deny per host
              2. Per-host bandwidth rate limiting (EGRESS, TC/HTB)
              3. Per-host site blocking using dnsmasq + nftables ipsets

-------------------------------------------------------------------------------
REQUIRED OPENWRT PACKAGES (Enable via: make menuconfig)
-------------------------------------------------------------------------------

Base system:
  - python3
  - python3-logging
  - python3-subprocess

Traffic Control (TC):
  - tc (iproute2)
  - kmod-sched-core
  - kmod-sched-htb
  - kmod-sched-fq-codel

Firewall / Netfilter:
  - firewall4 (fw4)
  - nftables
  - kmod-nft-core
  - kmod-nft-netdev

DNS / DHCP:
  - dnsmasq-full   (required for nftset support)

Optional (Debugging):
  - iperf3
  - tcpdump

-------------------------------------------------------------------------------
IMPORTANT DESIGN NOTES
-------------------------------------------------------------------------------
• Rate limiting is implemented using Linux TC (HTB + fq_codel)
• TC works ONLY on EGRESS traffic
• To limit client DOWNLOAD speed:
      → Apply TC on AP interface
      → Match packets using destination IP (ip dst)
• Upload shaping requires ingress policing (not implemented here)

-------------------------------------------------------------------------------
FILES USED
-------------------------------------------------------------------------------
JSON config        : /etc/hostcontrol/hostcontrol.json
DHCP leases        : /tmp/dhcp.leases
dnsmasq nft config : /etc/dnsmasq.d/ipsets.conf
Interface shaped   : phy0-ap0

-------------------------------------------------------------------------------
SAFE TO RUN MULTIPLE TIMES
-------------------------------------------------------------------------------
• Uses tc replace (idempotent)
• Removes duplicate filters
• Firewall rules are checked before adding

===============================================================================
"""

import json
import subprocess
import sys
import os

# ---------------------------------------------------------------------------
# CONFIGURATION
# ---------------------------------------------------------------------------
JSON_FILE = "/etc/hostcontrol/hostcontrol.json"
LEASES_FILE = "/tmp/dhcp.leases"
DNSMASQ_IPSETS_FILE = "/etc/dnsmasq.d/ipsets.conf"
LAN_IFACE = "phy0-ap0"  # Wi-Fi AP interface (EGRESS shaping)

# TC class ID tracking
MINOR_ID_START = 10
minor_id_counter = MINOR_ID_START
ip_to_minor = {}

# ---------------------------------------------------------------------------
# UTILITY FUNCTIONS
# ---------------------------------------------------------------------------
def run(cmd):
    """Execute shell command with error checking"""
    subprocess.run(cmd, shell=True, check=True)

def hostname_to_ip(hostname):
    """Resolve hostname to IP from DHCP leases"""
    with open(LEASES_FILE, "r") as f:
        for line in f:
            parts = line.split()
            if len(parts) >= 4 and parts[3] == hostname:
                return parts[2]
    return None

def rule_exists(name):
    """Check if firewall rule already exists"""
    try:
        subprocess.check_output(
            f"uci show firewall | grep \"option name='{name}'\"",
            shell=True, text=True
        )
        return True
    except subprocess.CalledProcessError:
        return False

def ipset_exists(name):
    """Check if firewall ipset already exists"""
    try:
        subprocess.check_output(
            f"uci show firewall | grep \"firewall.@ipset.*.name='{name}'\"",
            shell=True, text=True
        )
        return True
    except subprocess.CalledProcessError:
        return False

# ---------------------------------------------------------------------------
# FIREWALL / DNSMASQ FUNCTIONS
# ---------------------------------------------------------------------------
def ensure_ipset(setname):
    """Create nftables-compatible ipset"""
    if ipset_exists(setname):
        return
    run("uci add firewall ipset")
    run(f"uci set firewall.@ipset[-1].name='{setname}'")
    run("uci set firewall.@ipset[-1].match='dest_ip'")
    run("uci set firewall.@ipset[-1].family='ipv4'")

def ensure_block_rule(name, src_ip, setname):
    """Create firewall rule to block sites via ipset"""
    if rule_exists(name):
        return
    run("uci add firewall rule")
    run(f"uci set firewall.@rule[-1].name='{name}'")
    run("uci set firewall.@rule[-1].src='lan'")
    run(f"uci set firewall.@rule[-1].src_ip='{src_ip}'")
    run("uci set firewall.@rule[-1].dest='wan'")
    run("uci set firewall.@rule[-1].proto='all'")
    run(f"uci set firewall.@rule[-1].ipset='{setname}'")
    run("uci set firewall.@rule[-1].target='REJECT'")

def generate_dnsmasq_ipsets(blocked_sites):
    """Generate dnsmasq nftset configuration"""
    with open(DNSMASQ_IPSETS_FILE, "w") as f:
        for site, ipset_name in blocked_sites.items():
            f.write(f"nftset=/{site}/4#inet#fw4#{ipset_name}\n")

# ---------------------------------------------------------------------------
# TC (TRAFFIC CONTROL) FUNCTIONS
# ---------------------------------------------------------------------------
def ip_to_minor_id(ip):
    """Map IP address to unique TC class minor ID"""
    global minor_id_counter
    if ip in ip_to_minor:
        return ip_to_minor[ip]

    minor_id = minor_id_counter
    minor_id_counter += 1

    if minor_id > 65535:
        minor_id_counter = MINOR_ID_START
        minor_id = minor_id_counter

    ip_to_minor[ip] = minor_id
    return minor_id

def setup_tc_root():
    """Create root HTB qdisc and parent class if missing"""
    try:
        output = subprocess.check_output(
            f"tc class show dev {LAN_IFACE}",
            shell=True, text=True
        )
        if "1:1" in output:
            return
    except subprocess.CalledProcessError:
        pass

    run(f"tc qdisc del dev {LAN_IFACE} root 2>/dev/null || true")
    run(f"tc qdisc add dev {LAN_IFACE} root handle 1: htb default 999")
    run(
        f"tc class add dev {LAN_IFACE} parent 1: classid 1:1 "
        f"htb rate 300mbit ceil 300mbit"
    )

def ensure_tc_limit(ip, rate_mbps):
    """
    Apply per-host EGRESS rate limiting
    Matches packets by destination IP (client download)
    """
    minor_id = ip_to_minor_id(ip)

    run(
        f"tc class replace dev {LAN_IFACE} parent 1:1 "
        f"classid 1:{minor_id} htb "
        f"rate {rate_mbps}mbit ceil {rate_mbps}mbit"
    )

    run(
        f"tc qdisc replace dev {LAN_IFACE} parent 1:{minor_id} "
        f"handle {minor_id}: fq_codel"
    )

    run(
        f"tc filter del dev {LAN_IFACE} parent 1: "
        f"protocol ip pref 1 2>/dev/null || true"
    )

    run(
        f"tc filter add dev {LAN_IFACE} protocol ip parent 1: prio 1 u32 "
        f"match ip dst {ip}/32 flowid 1:{minor_id}"
    )

# ---------------------------------------------------------------------------
# MAIN
# ---------------------------------------------------------------------------
def main():
    if not os.path.exists(JSON_FILE):
        print("JSON file not found")
        sys.exit(1)

    with open(JSON_FILE) as f:
        data = json.load(f)

    blocked_sites = {}
    setup_tc_root()

    for entry in data.get("host_access_control", []):
        hostname = entry["hostname"]
        ip = hostname_to_ip(hostname)

        if not ip:
            print(f"Skipping {hostname}: IP not found")
            continue

        base_name = f"hostcontrol_{hostname}"

        # ---- TIME BASED DENY ----
        if entry["type"] == "time":
            rule_name = f"{base_name}_time"
            if rule_exists(rule_name):
                continue

            start = entry["time_window"]["start_time"][11:19]
            stop = entry["time_window"]["stop_time"][11:19]

            run("uci add firewall rule")
            run(f"uci set firewall.@rule[-1].name='{rule_name}'")
            run("uci set firewall.@rule[-1].src='lan'")
            run(f"uci set firewall.@rule[-1].src_ip='{ip}'")
            run(f"uci set firewall.@rule[-1].start_time='{start}'")
            run(f"uci set firewall.@rule[-1].stop_time='{stop}'")
            run("uci set firewall.@rule[-1].proto='all'")
            run("uci set firewall.@rule[-1].target='REJECT'")

        # ---- RATE LIMIT ----
        elif entry["type"] == "rate":
            ensure_tc_limit(ip, 50)

        # ---- SITE BLOCK ----
        elif entry["type"] == "siteaccess":
            for site in entry.get("blocked_sites", []):
                ipset_name = f"{site.replace('.', '_')}_{hostname}"
                blocked_sites[site] = ipset_name
                ensure_ipset(ipset_name)
                ensure_block_rule(f"block_{site}_{hostname}", ip, ipset_name)

    run("uci commit firewall")
    run("/etc/init.d/firewall reload")

    generate_dnsmasq_ipsets(blocked_sites)

    run("uci set dhcp.@dnsmasq[0].confdir='/etc/dnsmasq.d'")
    run("uci commit dhcp")
    run("/etc/init.d/dnsmasq restart")

    print("Time Deny, Site Blocking, and Rate Limiting applied successfully")

if __name__ == "__main__":
    main()

