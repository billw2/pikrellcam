#!/usr/bin/python
import sys

output = "terminal"

if len (sys.argv) < 2 :
	print("Usage: " + sys.argv[0] + " [C|F] {fifo}")
	print("    C or F must be given")
	print("    If \"fifo\" arg is present, write to PiKrellCam FIFO")
	sys.exit(1)

mode = sys.argv[1]
if (mode != "F") and (mode != "C"):
	print("Usage: " + sys.argv[0] + " [C|F] {fifo}")
	print("    C or F must be given")
	print("    If \"fifo\" arg is present, write to PiKrellCam FIFO")
	sys.exit(1)

if len (sys.argv) == 3 :
	output = sys.argv[2]

from os.path import expanduser
home = expanduser("~")

# Edit to get labels in the temperature output
#label = ["T0=", "T1=", "T2=", "T3="]
label = ["", "", "", ""]

def read_temp(device):
	global mode
	for count in range(0, 3):
		path = "/sys/bus/w1/devices/" + device.rstrip() + "/w1_slave"
		with open(path) as file:
			lines = file.read()

		line1 = lines.split("\n")[0]
		line2 = lines.split("\n")[1]
		crc = line1.split(" ")[-1]
		temp = line2.split("=")[1]
		if crc == "YES":
			break

	if crc == "YES":
		if mode == "F" :
			t = float(temp) / 1000.0 * 9.0 / 5.0 + 32
		else:
			t = float(temp) / 1000.0
	else:
		t = 0.0
	return t

try:
	id = 0
	if output == "fifo":
		sep = "_"
	else:
		sep = " "
	out_string = ""
	devices = open("/sys/bus/w1/devices/w1_bus_master1/w1_master_slaves")
	for device in devices.readlines():
		t = read_temp(device)
		out_string = sep + out_string + label[id] + "{:.1f}".format(t) + mode
		id += 1
	devices.close()
	if output == "fifo":
#		fifo = open(home + "/pikrellcam/www/FIFO", "w")
		fifo = open("/home/pi/pikrellcam/www/FIFO", "w")
		fifo.write("annotate_string append ds18b20 " + out_string + "\n")
		fifo.close()
	else:
		print(out_string)
except:
	print("no devices")
