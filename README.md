# Network_FinalProject

## Routing Protocol Implementation

### In this implementation we develop a node in a network where communication between different nodes is possible. The nodes do not necessarily communicate directly with each other, and they can communicate through other nodes.

The following actions can be performed:

⚪ setid,id - setting the id of the current node.

⚪ connect,ip:port - connecting the current node to the node that meets the inputed IP and Port.

⚪ send,dest-node-id,data-length,data - sending data of the given length to the given destination node.

⚪ route,node-id - print the route to the given node if the node exists (if the route is unknown discover it before).

⚪ peers - printing the list of the connected (directly) nodes.

The methods implemented in this class:
std_setid(#param) - determines the current node ID based on the input.

std_connect(#param) - opening a new socket and sending a request to connect to the IP and the port received at the input. 
(necessary condition - ID is defined in both nodes).
std_send(#param) - sending a message of a certain length to the node given in the input.
 If the path to the node unknown, a DISCOVER operation is activated (as explained below) before sending.
std_route(#param) - prints the best route to the node given in the input. If the path to the node is not known,
 the DISCOVER operation is activated (as explained below) before printing the route.
std_peers(#param) - prints the neighboring nodes (directly connected).
gotmsg(#param) - using a switch case when receiving a message to redirect to corresponding incoming message function.
input_ack(#param) - handles the case of incoming ack message (depending on the source message on which the ACK message was returned).
input_nack(#param) - handles the case of incoming nack message (depending on the source message on which the ACK message was returned).
input_connect(#param) - accepting SOCKET login (if ID has been placed on both sides) and then adding the SOCKET to the list of neighbors.
input_send(#param) - print out the input message and sends an ack message back to sender.
input_discover(#param) - flooding DISCOVER messages to the network for the purpose of finding the best route to a particular node. Intuitive explanation: 1. sending a DISCOVER message to all the vertices (a DISCOVER message is sent to a single neighbor at a time until a reply is returned, then sent to the next neighbor and so on). 2. If a DISCOVER message has been sent to a leaf node or a node we have already visited (closing a circle) - returns a NACK message back to the last node from which a DISCOVER message was sent. If the destination node is found - return the route by ROUTE message.
input_route(#param) - if there are neighbors for whom a DISCOVER operation has not been performed - continue to perform a DISCOVER operation in order to find the best route. Otherwise, the route is saved in the "waze" data structure (if an internal node has received this message, the saving is performed in a temporary "checkpoint_waze" data structure). If there is already a route - the best route is maintained.
input_relay(#param) - receiving a RELAY message, receiving the continued chaining of the messages (explained below) in the input and sending it to the next vertex in the route.
input_refresh(#param) - flooding REFRESH messages to the network when the topology has been changed (node deleted/added to the network) in order to delete existing routes in each node. the algorithm is implemented in a similar way to the DISCOVER algorithm explained above.
send_route(#param) - this function is used to respond to a DISCOVER message when finding a route. creates the route and sends it back to the node that send the discover message.
send_nack(#param) - sending a NACK message in response to the message in the input (depending on the type of message).
send_ack(#param) - sending a ACK message in response to the message in the input (depending on the type of message).
send_discover(#param) - sending a DISCOVER message in response to the parameters in the input.
send_relay(#param) - a call to this function is made when there is a path to the target node. the function chains RELAY messages with a SEND message where each RELAY message is intended for an internal node in the track and the SEND message (containing the content) to the destination node. if the content of the message exceeds one SEND message, several SEND messages will be chained.
send_refresh(#param) - sending a REFRESH message in response to the parameters in the input.
'Select' (Implements listening to sockets):
In this class the methods help to listen to the user input and also to listen to different sockets in parallel.

The following actions can be performed:

Add fd to monitoring - Add fd to listening list.
Remove fd from monitoring - Remove fd from listening list.
Wait for input - Waiting for input from the user or input from one of the sockets from the listening list. When receiving a certain input the function returns the fd number.

  
  ## UDP IPv4 VS IPv6
Diagnosing the difference between the HEADER of IPV4 versus IPV6 under UDP protocol.


  Different between ipv4 lipv6:
• IPv6 is much simpler than IPv4 - the size of IPv6 is much larger than that of IPv4, because of the address size
IPv6. The IPv4 addresses have 32-bit (decimal) binary numbers, and the IPv6 addresses have
Binary numbers of 128 bits) hexadecimal. In IPv4 the address is both the source and the destination
 .) Source and destination
• • The fields in the IPv4 header such as Length Header Internet (IHL, ID, existing existing flags)
In the IPv6 header.
Fragmentation (facts, IPv6 as given in IPv4) cannot be split
• • (TTL (Live-to-Time), a field in the IPv4 header, commonly used to prevent a routing signal coop, is called
To its exact meaning, "Limit Hop."
• In IPv4 there is Checksum compared to IPv6 which does not have a faulty Pact, it saves processing time and speeds up the
Transfer of facts.
