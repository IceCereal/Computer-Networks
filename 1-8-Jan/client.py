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

print (s.recv(1024).decode("utf-8"))

# Task - 1
s.send(b'It was nice talking to you!')

# Task - 2
from random import randint
N = randint(25, 100)

array = [randint(1, 100) for i in range(N)]

s.send(bytes(str(array).encode("utf-8")))

print ("Original List:\t", array)
print ("Deduplicated List:\t", s.recv(1024).decode("utf-8"))
print ("Duplicates List:\t", s.recv(1024).decode("utf-8"))

s.close()