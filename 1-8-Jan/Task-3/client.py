import socket

# The socket Instance
s = socket.socket()

# Local Machine Name
host = socket.gethostname()

# Server Port
portServer = 3010

# Client Port (for incoming connections)
portClient = 1337

s.bind((host, portClient))
s.connect((host, portServer))

# Confirmation
print (s.recv(1024).decode("utf-8"))

from random import randint
N = randint(25, 100)

array = [randint(80, 100) for i in range(N)]

# Send Client List
s.send(bytes(str(array).encode("utf-8")))

print ("Original List:\t", array)
print ("Sorted List:\t", s.recv(1024).decode("utf-8"))
print ("Common Numbers List:\t", s.recv(1024).decode("utf-8"))

s.close()