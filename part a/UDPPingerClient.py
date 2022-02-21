from socket import *
from datetime import datetime

NUM_PINGS = 10

serverName = '127.0.0.1'
serverPort = 12000

clientSocket = socket(AF_INET, SOCK_DGRAM)
clientSocket.settimeout(1)

packets_received = 0
packets_lost = 0
rtt = []

for ping_sequence in range(NUM_PINGS):
    try:

        time_when_sent = datetime.now()
        message = ' ping: ' + str(ping_sequence) + ' ' + str(time_when_sent)
        clientSocket.sendto(message.encode(), (serverName, serverPort))

        data, serverAddress = clientSocket.recvfrom(1024)
        len_d = len(data)
        if data:
            print(data)
        print(str(len_d) + " bytes " + "from " + serverName + ":" + str(serverPort) + ' ')
        round_trip_time = (datetime.now() - time_when_sent).total_seconds()
        print("RTT: %s seconds \n" % round_trip_time)

        packets_received += 1
        rtt.append(round_trip_time)

    except timeout:
        print("Request timed out. ")
        packets_lost += 1

percentage_lost = int(100 * (packets_lost / NUM_PINGS))

clientSocket.close()
print("Closing socket. \n")

print("Ping statistics for %s: \n"
      "\tPackets: Sent = %s, Received = %s, Lost = %s (%s%% Loss)" % (serverName, NUM_PINGS, packets_received,
                                                                      packets_lost, percentage_lost))

avg_rtt = sum(rtt) / len(rtt)

print("Approximate round-trip times in seconds: \n"
      "\t Maximum = %s, Minimum = %s, Average = %s" % (max(rtt), min(rtt), avg_rtt))