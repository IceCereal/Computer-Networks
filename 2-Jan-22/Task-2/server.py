import socket
from random import randint
from ast import literal_eval as l_e
from time import sleep

# The socket Instance
s = socket.socket(socket.AF_INET, socket.SOCK_STREAM, 0)

# Local Machine Name
host = socket.gethostbyname(socket.gethostname())

# Choose port & bind it
port = 3011
s.bind((host, port))

print ("host ip:port:\t", host, port)

# Listen for client connections. The number
# it specifies the number of unaccepted connections that the system
# will allow before refusing newconnections.
s.listen(5)

while True:
	# Establish a connection with the client
	cli, addr = s.accept()

	# Yay!
	print ("Got connection from", addr)

	# Initial Prompt
	"""
		Type of Service:
			0 - deduplication + [___]
			1 - intersection + [1___]
	"""
	prompt = int(cli.recv(1024).decode("utf-8"))

	if prompt == 0:
		Client_List = cli.recv(1024).decode("utf-8")
		List = l_e(Client_List)
		ListDup = [i for i in List]

		print ("clientList (unsorted):\t", List)

		len_list = 0

		for element in List:
			len_list += 1

		serverList = [randint(1, 100) for i in range(len_list)]

		List.sort()

		ListDup = set(ListDup)
		serverList = set(ListDup)

		commonNumbers = []

		for i in ListDup:
			if i in serverList:
				commonNumbers.append(i)

		cli.send(bytes(str(List).encode("utf-8")))
		sleep(0.25)

		print ("serverList:\t", list(serverList))
		print ("clientList (sorted):\t", List)
		print ("commonList:\t", commonNumbers)

		cli.send(bytes(str(commonNumbers).encode("utf-8")))
	else:
		Client_List = cli.recv(1024).decode("utf-8")
		List = l_e(Client_List)

		print ("clientList (unsorted):\t", List)

		len_list = 0

		for element in List:
			len_list += 1

		serverList = [randint(1, 100) for i in range(len_list)]

		print ("serverList:\t", serverList)

		List = set(List)
		serverList = set(List)

		intersectedList = list(List.intersection(serverList))

		print ("intersectedList", intersectedList)

		cli.send(bytes(str(intersectedList).encode("utf-8")))

	cli.close()