# Network_FinalProject

## Routing Protocol Implementation

### In this implementation we develop a node in a network where communication between different nodes is possible. The nodes do not necessarily communicate directly with each other, and they can communicate through other nodes.

The following actions can be performed:
⚪ setid,id - setting the id of the current node.
⚪ connect,ip:port - connecting the current node to the node that meets the inputed IP and Port.
⚪ send,dest-node-id,data-length,data - sending data of the given length to the given destination node.
⚪ route,node-id - print the route to the given node if the node exists (if the route is unknown discover it before).
⚪ peers - printing the list of the connected (directly) nodes.
