#include "node.hpp"

Node::Node(int port): _nodeData(port)
{
    int optVal = 1;
    struct sockaddr_in servaddr{};

    if(0 > (_nodeData.socket = socket(AF_INET, SOCK_STREAM, TCP_PROTOCOL))) /* ipv4, tcp, default protocol */
    {
      throw std::runtime_error("socket creation failed\n");
    }

    memset(&servaddr, 0, sizeof(servaddr));

    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);  /* will bind the socket to any available interfaces */
    servaddr.sin_port = htons(_nodeData.port);

    /* clear the port for the listening socket */
    if(0 > setsockopt(_nodeData.socket, SOL_SOCKET, SO_REUSEADDR, &optVal, sizeof(optVal)))
    {
        throw std::runtime_error("port reuse failed\n");
    }

    /* bind with the listening socket */
    if (0 > bind(_nodeData.socket, (struct sockaddr *)&servaddr, sizeof(servaddr)))
    {
        throw std::runtime_error("bind failed\n");
    }

    add_fd_to_monitoring(INPUT_CHANNEL);
}

void Node::setid(int nodeId) noexcept(false)
{
    _nodeData.id = nodeId;
    if(!_isSetId)
    {
        _isSetId = true;
        add_fd_to_monitoring(_nodeData.socket); // add listening socket (for new connections)

        if (0 > listen(_nodeData.socket, MAX_QUEUE_SIZE))
        {
            throw std::runtime_error("listen failed\n");
        }
    }
}

void Node::connectToNode(const std::string& destIpAddress, int destPort)
{
    if(!_isSetId)
    {
        throw std::runtime_error("error: cannot connect to another node before your id is set\n");
    }

    int sockfd;
    struct sockaddr_in servaddr{};

    // socket create and verify
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1) {
        std::cout << "nack" << std::endl;
        return;
    }

    bzero(&servaddr, sizeof(servaddr));
    // assign IP, PORT of the server address params
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = inet_addr(destIpAddress.c_str());
    servaddr.sin_port = htons(destPort);

    // connect the client socket to server socket
    if (connect(sockfd, (struct sockaddr*)&servaddr, sizeof(servaddr)) < 0) {
        std::cout << "nack" << std::endl;
        return;
    }

    auto connectMsg = MessageGenerator::generateConnectMessage(getNextMsgNum(), _nodeData.id);
    if (send(sockfd, &connectMsg, sizeof(connectMsg), 0) < 0)
    {
        std::cout << "nack" << std::endl;
        return;
    }

    Message m = {};
     if (recv(sockfd, &m, sizeof(m), 0) == -1)
    {
        std::cout << "nack" << std::endl;
        return;
    }

    // check message params
    int payloadMsgAck;
    memcpy(&payloadMsgAck, m.payload, 4);
    if(m.destId != _nodeData.id || m.func != Ack || payloadMsgAck != connectMsg.msgId)
    {
        std::cout << "nack" << std::endl;
        return;
    }

    NodeData neighborData(destPort, sockfd, m.sourceId ,destIpAddress );
    _neighbors.insert({neighborData.id, neighborData});
    std::cout << "ack" << std::endl;

    add_fd_to_monitoring(sockfd);
}


void Node::peers()
{
    if(_neighbors.empty())
        return;

    const auto lastId = std::prev(_neighbors.end())->first;
    for(const auto& [id, value] : _neighbors)
    {
        std::cout << id;
        if(id != lastId)
        {
            std::cout << ",";
        }
        else
        {
            std::cout << std::endl;
        }
    }
}


std::string Node::getIpAddress()
{
    struct ifaddrs *ifap, *ifa;
    struct sockaddr_in *sa;

    getifaddrs (&ifap);
    for (ifa = ifap; !ifa->ifa_addr || ifa->ifa_addr->sa_family != AF_INET; ifa = ifa->ifa_next);
    sa = (struct sockaddr_in *) ifa->ifa_addr;
    freeifaddrs(ifap);
    return inet_ntoa(sa->sin_addr);
}

int Node::getNodeId() const
{
    if(!_isSetId)
    {
        throw std::runtime_error("error: node id wasn't set yet\n");
    }
    return _nodeData.id;
}


int Node::getNextMsgNum() noexcept(false)
{
    if(!_isSetId)
    {
        throw std::runtime_error("cannot generate msg num for uninitialized node Id\n");
    }
    return _nodeData.id * 10000 + msgCounter++;
}