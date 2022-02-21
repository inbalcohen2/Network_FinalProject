# UDPPingerServer.py
# We will need the following module to generate randomized lost packets
import random
import time
from socket import *

# Create a UDP socket
# Notice the use of SOCK_DGRAM for UDP packets
serverSocket = socket(AF_INET, SOCK_DGRAM)
# Assign IP address and port number to socket
serverSocket.bind(('', 12000))
counter = 1
x = 5

while True:
    print('Ready to receive pings')
    # Generate random number in the range of 0 to 10
    rand = random.randint(0, 10)
    # Receive the client packet along with the address it is coming from
    message, address = serverSocket.recvfrom(1024)
    # Capitalize the message from the client
    message = message.upper()
    messageFromClient = str(message).split(' ')
    time_last_message = time.time()

    start_time = messageFromClient[2]
    seq_num = messageFromClient[1]
    prev_seq = 0


    total_time = time.time() - float(start_time)

    if time.time() - time_last_message > x:
        packets_lost = prev_seq - int(seq_num) + 1
        prev_seq = messageFromClient[1]
        print("client discount")
        print(' packets lost: ', packets_lost)
        exit(0)

    # if time is between 1 to 4 seconds - received
    else:
        message = 'Heartbeat ' + str(counter) + ' is received'
        print(message)
        serverSocket.sendto(message.encode(), address)

    counter += 1
