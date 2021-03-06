import socket
from random import randint
from ast import literal_eval as le
from time import sleep

# The socket Instance
s = socket.socket()

# Local Machine Name
host = socket.gethostname()

# Server Port
portServer = 3010

# Client Port (for incoming connections)
portClient = 1337

s.bind((host, portClient))
s.connect(("10.59.102.66", portServer))

N = 80 # Number of elements (Max: 999)
if N > 999:
	print ("N cannot exceed 999...\nExiting")
	exit()

"""
	Type of Service:
		0 - deduplication
		1 - intersection
"""
SERVICE = {
	"deduplication" : 0,
	"intersection" : 1
}

service = SERVICE["intersection"]

if service == 0:
	s.send(bytes(str(1).encode("utf-8")))
	sleep(0.25)

	array = [randint(1, 100) for i in range(N)]

	s.send(bytes(str(array).encode("utf-8")))

	dedup_list_raw = s.recv(2048).decode("utf-8")
	dedup_list = le(dedup_list_raw)

	print ("Deduplicated List:\t", dedup_list[0])
	print ("Common Numbers:\t", dedup_list[1])

elif service == 1:
	s.send(bytes(str(2).encode("utf-8")))
	sleep(0.25)

	array = [randint(1, 100) for i in range(N)]

	s.send(bytes(str(array).encode("utf-8")))

	intersected_list_raw = s.recv(1024).decode("utf-8")
	intersected_list = le(intersected_list_raw)

	print ("Intersected List:\t", intersected_list)

s.close()