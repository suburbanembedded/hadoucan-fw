#!/usr/bin/env python2

import os
import sys
import shutil
import serial
#import argparse

def fastboot_sync(tty_port):

	print 'Starting Sync'

	tty_port.write('\r')
	tty_port.flush()

	tty_port.timeout = 1

	temp = tty_port.read_until(size=128)
	while not temp:
		temp = tty_port.read_until(size=128)

	tty_port.timeout = 10

	print 'Sync Complete'


def fastboot_send_file(tty_port, infile, outname):
	img_info = os.stat(infile)

	download_cmd = 'download:{0:08x}\r'.format(img_info.st_size)
	download_resp = 'DATA{0:08x}'.format(img_info.st_size)
	flash_cmd    = 'flash:{0}\r'.format(outname)
	ok_resp      = 'OKAY'

	print 'Starting Download'
	tty_port.write(download_cmd)
	tty_port.flush()
	tty_line = tty_port.read_until(size=12)

	if tty_line != download_resp:
		print 'lost dl start ok, expected {0} got {1}'.format(download_resp, tty_line)
		sys.exit(-1)

	img_file = open(infile, 'rb')
	img_line = img_file.readline()

	while img_line:
		tty_port.write(img_line)
		img_line = img_file.readline()

	img_file.close()

	tty_port.flush()

	tty_line = tty_port.read_until(size=4)
	if tty_line != ok_resp:
		print 'lost dl finish ok, got {0}'.format(tty_line)
		sys.exit(-1)

	print 'Download Complete'

	print 'Starting Flash'

	tty_port.write(flash_cmd)
	tty_port.flush()

	tty_line = tty_port.read_until(size=4)
	if tty_line != ok_resp:
		print 'lost flash ok, got {0}'.format(tty_line)
		sys.exit(-1)

	print 'Flash complete'


def main():
	
	# parser = argparse.ArgumentParser(description='Send new firmware to a SM-1301')

	# parser.add_argument('--img', dest='img_path', action='store',
	# 	default='app.bin.enc',
	# 	help='Path to the encrypted firmwware image')

	# parser.add_argument('--aux', dest='aux_img_path', action='store',
	# 	default='app.bin.enc.xml',
	# 	help='Path to the aux file')

	# parser.add_argument('--tty', dest='tty_path', action='store',
	# 	default='/dev/ttyACM0',
	# 	help='A path to a serial port')

	# args = parser.parse_args()

	img_path     = 'build/ram/debug/hadoucan-fw/app.bin.enc'
	aux_img_path = 'build/ram/debug/hadoucan-fw/app.bin.enc.xml'
	tty_path = '/dev/ttyACM0'

	tty_port = serial.Serial(tty_path, 115200, timeout=10)

	print 'Device open'

	fastboot_sync(tty_port)

	fastboot_send_file(tty_port, img_path, 'app.bin.enc')
	fastboot_send_file(tty_port, aux_img_path, 'app.bin.enc.xml')

	sys.exit(0)


main()