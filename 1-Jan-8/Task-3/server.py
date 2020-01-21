import socket
from random import randint
from ast import literal_eval as l_e

# The socket Instance
s = socket.socket(socket.AF_INET, socket.SOCK_STREAM, 0)

# Local Machine Name
host = socket.gethostbyname(socket.gethostname())

# Choose port & bind it
port = 3010
s.bind(('0.0.0.0', port))

print ("host ip:port:\t", host, port)

# Listen for client connections. The number
# it specifies the number of unaccepted connections that the system
# will allow before refusing newconnections.
s.listen(5)

# Task - 3
while True:
	# Establish a connection with the client
	cli, addr = s.accept()

	# Yay!
	print ("Got connection from", addr)

	# Confirmation message
	cli.send(b'Thank you for connecting!')

	Client_List = cli.recv(1024).decode("utf-8")
	List = l_e(Client_List)
	ListDup = [i for i in List]

	print ("clientList (unsorted):\t", List)

	serverList = [randint(1, 100) for i in range(len(List))]

	List.sort()

	ListDup = set(ListDup)
	serverList = set(ListDup)

	commonNumbers = []

	for i in ListDup:
		if i in serverList:
			commonNumbers.append(i)

	cli.send(bytes(str(List).encode("utf-8")))

	print ("serverList:\t", list(serverList))
	print ("clientList (sorted):\t", List)
	print ("commonList:\t", commonNumbers)

	cli.send(bytes(str(commonNumbers).encode("utf-8")))

	cli.close()
