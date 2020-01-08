import socket
from ast import literal_eval as l_e

# The socket Instance
s = socket.socket(socket.AF_INET, socket.SOCK_STREAM, 0)

# Local Machine Name
host = socket.gethostbyname(socket.gethostname())

# Choose port & bind it
port = 3010
s.bind((host, port))

print ("host ip:port:\t", host, port)

# Listen for client connections. The number
# it specifies the number of unaccepted connections that the system
# will allow before refusing newconnections.
s.listen(5)

while True:
	# Establish a connection with the client
	c, addr = s.accept()

	# Yay!
	print ("Got connection from", addr)

	# Confirmation message
	c.send(b'Thank you for connecting')

	# Task - 1
	print (c.recv(1024).decode("utf-8"))

	# Task - 2
	rawList = c.recv(1024).decode("utf-8")
	List = l_e(rawList)
	Deduplicated = []
	Duplicates = []

	for i in range(len(List)):
		if List.index(List[i]) == i:
			Deduplicated.append(List[i])
		else:
			Duplicates.append(List[i])

	c.send(bytes(str(Deduplicated).encode("utf-8")))
	c.send(bytes(str(Duplicates).encode("utf-8")))

	# Close the connection
	c.close()