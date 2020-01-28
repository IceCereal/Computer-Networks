import socket
from random import randint
from ast import literal_eval as le
from time import sleep

# The socket Instance
s = socket.socket()

# Local Machine Name
host = socket.gethostname()

# Server Port
portServer = 3011

# Client Port (for incoming connections)
portClient = 13371

s.bind((host, portClient))
s.connect((host, portServer))

N = 30 # Number of elements (Max: 999)
if N > 999:
	print ("N cannot exceed 999...\nExiting")
	exit()

"""
	Type of Service:
		0 - deduplication
		1 - intersection
"""
service = 1

if service == 0:
	s.send(bytes(str(N).encode("utf-8")))
	sleep(0.25)

	array = [randint(1, 100) for i in range(N)]

	s.send(bytes(str(array).encode("utf-8")))

	dedup_list_raw = s.recv(1024).decode("utf-8")
	dedup_list = le(dedup_list_raw)

	print ("Deduplicated List:\t", dedup_list)

	commonNumbers_raw= s.recv(1024).decode("utf-8")
	commonNumbers = le(commonNumbers_raw)

	print ("Common Numbers:\t", commonNumbers)

elif service == 1:
	s.send(bytes(str(1000 + N).encode("utf-8")))
	sleep(0.25)

	array = [randint(1, 100) for i in range(N)]

	s.send(bytes(str(array).encode("utf-8")))

	intersected_list_raw = s.recv(1024).decode("utf-8")
	intersected_list = le(intersected_list_raw)

	print ("Intersected List:\t", intersected_list)

s.close()