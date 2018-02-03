#!/usr/bin/python

import argparse
import subprocess
import re
import time
import signal
import sys
from python_arptable import get_arp_table
from collections import Counter

# Al Nintendo devices
mac_prefixes = [
"E8:4E:CE",
"E0:E7:51",
"E0:0C:7F",
"DC:68:EB",
"D8:6B:F7",
"CC:FB:65",
"CC:9E:00",
"B8:AE:6E",
"A4:C0:E1",
"A4:5C:27",
"9C:E6:35",
"98:B6:E9",
"8C:CD:E8",
"8C:56:C5",
"7C:BB:8A",
"78:A2:A0",
"64:B5:C6",
"5C:52:1E",
"58:BD:A3",
"40:F4:07",
"40:D2:8A",
"34:AF:2C",
"2C:10:C1",
"18:2A:7B",
"04:03:D6",
"00:27:09",
"00:26:59",
"00:25:A0",
"00:24:F3",
"00:24:44",
"00:24:1E",
"00:23:CC",
"00:23:31",
"00:22:D7",
"00:22:AA",
"00:22:4C",
"00:21:BD",
"00:21:47",
"00:1F:C5",
"00:1F:32",
"00:1E:A9",
"00:1E:35",
"00:1D:BC",
"00:1C:BE",
"00:1B:EA",
"00:1B:7A",
"00:1A:E9",
"00:19:FD",
"00:19:1D",
"00:17:AB",
"00:16:56",
"00:09:BF"
]

def get_3ds_mac_addrs():
	"""
	Filters devices connected to local access point, using hostapd_cli utility and searching for Nintendo manufactured devices.
	Note: it's currently impossible to distinguish between different Nintendo consoles since they share MAC addresses ranges.
	"""
	addrs = []
	out = subprocess.check_output(['hostapd_cli', 'all_sta'])

	matches = re.finditer(r'^dot11RSNAStatsSTAAddress=(('+'|'.join(mac_prefixes)+').*?)$', out, re.M|re.I)
	for m in matches:
		addrs.append(m.group(1))
	# MAC addrs are returned in reverse order, we want them in the order devices were connected
	return list(reversed(addrs))


def get_ip_addrs_for_mac(mac_addrs):
	"""
	Gets IP addresses from MAC addresses using ARP table.
	"""
	addrs = []
	for ma in mac_addrs:
		for entry in get_arp_table():
			if entry['HW address'] == ma:
				addrs.append(entry['IP address'])
	return addrs


def ip_addrs_equals(n_addrs, o_addrs):
	"""
	Compare addresses lists ignoring order.
	"""
	return Counter(n_addrs) == Counter(o_addrs)

def write_addrs_to_file(addrs, filename="g3ir.list"):
	"""
	Writes list of IP addresses to file, one per line.
	"""
	f = open(filename, 'w')
	for a in addrs:
		f.write(a + "\n")

def do_scan(filename="g3ir.list", old_addrs=[]):
	addrs = get_ip_addrs_for_mac(get_3ds_mac_addrs())
	if ip_addrs_equals(addrs, old_addrs):
		return False
	write_addrs_to_file(addrs, filename)
	return addrs

def start_scan_loop(delay=5, filename="g3ir.list"):
	"""
	Starts devices discovery performing at a rate of delay seconds.
	"""
	old_addrs = []
	while (True):
		res = do_scan(filename, old_addrs)
		if res != False:
			old_addrs = res
			print("Addresses changed.")
		time.sleep(delay)

def signal_handler(signum, frame):
	if signum == signal.SIGINT:
		print("Interrupt detected. Exiting.")
		sys.exit(0)

signal.signal(signal.SIGINT, signal_handler)

parser = argparse.ArgumentParser(description="Get IP addresses of connected Nintendo 3DS devices.")
parser.add_argument("-d", "--delay", type=int, default=5, help="Delay between one check and the next in seconds")
parser.add_argument("-o", "--output", type=str, default="g3ir.list", help="Output to the specified file")

args = parser.parse_args()

start_scan_loop(args.delay, args.output)
