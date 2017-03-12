#!/usr/bin/env python 
#
#
# A simple time syncing daemon. Listens for the arduino to send a special 
# request byte via serial, and responds with a timezone stamp to sync the 
# clock.

import serial
import time
import datetime

ser = serial.Serial('/dev/ttyACM0', 9600)
syncByte = "T";
timeZone = -5
while True:
	request = ser.readline().strip('\r\n');
	if request == syncByte:
		d = int(datetime.datetime.now().strftime("%s"));
		t = 60*60*timeZone;
		dt = "T"+str(d+t)+'\n';
		ser.write(dt);
	print request;
