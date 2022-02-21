#include "communicationCenter.hpp"

using namespace std;

void CommunicationCenter::SendMessage(Node& currentNode, int destNodeId, int msgLength, const std::string& msgToSend)
{
    const auto msg = MessageGenerator::generateSendMessage
            (currentNode.getNextMsgNum(), currentNode.getNodeId(), destNodeId, msgLength, msgToSend);

    const auto& neighbors = currentNode.getNeighbors();

    // send if connected directly
    if(neighbors.find(destNodeId) != neighbors.end())
    {
        auto socketFd = neighbors.at(destNodeId).socket;
        if (send(socketFd, &msg, sizeof(msg), 0) < 0)
        {
            throw runtime_error("sending directly send message failed\n");
        }
    }

    // if not connected directly, send discover message to all neighbors
    // Note: to prevent cycles, Discover saves in the payload all visited nodes. means:
    // payload: [searched-id][lengthVisited][visitedId1, visitedId2, ...]
    else
    {
        DiscoverObject dscObj;
        for(const auto &n : neighbors)
        {
            const std::vector<int> visitedNodesIds = {currentNode.getNodeId()};
            int dscMsgNum = currentNode.getNextMsgNum();
            const auto discoverMessage = MessageGenerator::generateDiscoverMessage
                    (dscMsgNum, currentNode.getNodeId(), n.first , destNodeId, 1, visitedNodesIds);

            if (send(n.second.socket, &discoverMessage, sizeof(discoverMessage), 0) < 0)
            {
                throw runtime_error("sending discover message to neighbors failed\n");
            }
            dscObj.pendingDscMsgIds.insert(dscMsgNum);
        }
        _dscObjs[ORIGINAL_STARTED] = dscObj;
    }
}

void CommunicationCenter::printRoute(Node &node, int destNodeId)
{
    if(node.getNodeId() == destNodeId)
    {
        cout << destNodeId << endl;
        return;
    }

    const auto& neighbors = node.getNeighbors();

    // print if connected directly
    if(neighbors.find(destNodeId) != neighbors.end())
    {
        cout << node.getNodeId() << "," << destNodeId << endl;
    }

    // if not connected directly, send discover message to all neighbors
    // Note: to prevent cycles, Discover saves in the payload all visited nodes. means:
    // payload: [searched-id][lengthVisited][visitedId1, visitedId2, ...]
    else
    {
        DiscoverObject dscObj;
        for(const auto &n : neighbors)
        {
            const std::vector<int> visitedNodesIds = {node.getNodeId()};
            int dscMsgNum = node.getNextMsgNum();
            const auto discoverMessage = MessageGenerator::generateDiscoverMessage
                    (dscMsgNum, node.getNodeId(), n.first , destNodeId, 1, visitedNodesIds);

            if (send(n.second.socket, &discoverMessage, sizeof(discoverMessage), 0) < 0)
            {
                throw runtime_error("sending discover message to neighbors failed\n");
            }
            dscObj.pendingDscMsgIds.insert(dscMsgNum);
        }
        _dscObjs[ORIGINAL_STARTED] = dscObj;
    }
}

/* returns new neighbor port */
void CommunicationCenter::onReceivedConnectionRequest(Node& currentNode)
{
    int connectionSocketFd;
    struct sockaddr_in cliaddr{};
    memset(&cliaddr, 0, sizeof(cliaddr));
    socklen_t len = sizeof(struct sockaddr_in);

    /* accept the first client from the queue */
    if (0 > (connectionSocketFd = accept(currentNode.getListeningSocket(), (struct sockaddr *)&cliaddr, &len)))
    {
        cerr << "error: accept failed." << endl;
    }

    Message m = {};
    if (recv(connectionSocketFd, &m, sizeof(m), 0) == -1)
    {
        cerr << "error: receive failed." << endl;
    }

    NodeData clientData
    ((int) ntohs(cliaddr.sin_port), connectionSocketFd, m.sourceId, inet_ntoa(cliaddr.sin_addr));
    currentNode.addNeighbor(clientData);

    auto ackMsg = MessageGenerator::generateAckMessage
            (currentNode.getNextMsgNum(), currentNode.getNodeId(), m.sourceId, m.msgId);
    if (send(connectionSocketFd, &ackMsg, sizeof(ackMsg), 0) < 0)
    {
        cerr << "error: sending ack failed." << endl;
    }

    add_fd_to_monitoring(connectionSocketFd);
}

void CommunicationCenter::onMessageReceived(Node& currentNode, Message &msg)
{
    switch(msg.func)
    {
        case Ack:
            cout << "ack" << endl;
            break;
        case Nack:
            onNackMessageReceived(currentNode, msg);
            cout << "nack" << endl;
            break;
        case Discover:
            onDiscoverMessageReceived(currentNode, msg);
            break;
        case Route:
            onRouteMessageReceived(currentNode, msg);
            break;
        case Send:
            onSendMessageReceived(currentNode, msg);
            break;
        default:
            throw invalid_argument("unsupported func command\n");
    }
}

void CommunicationCenter::onNackMessageReceived(Node& currentNode, Message &msg)
{
    int correspondDscMsg;
    memcpy(&correspondDscMsg, msg.payload, sizeof(int));

    int originalMsgId = -1;
    for(const auto& [key, obj] : _dscObjs)
    {
        if(obj.pendingDscMsgIds.find(correspondDscMsg) != obj.pendingDscMsgIds.end())
        {
            originalMsgId = key;
            break;
        }
    }

    if(originalMsgId == -1)
    {
        throw invalid_argument("no such discover msg which suits the route msg payload\n");
    }

    _dscObjs.at(originalMsgId).pendingDscMsgIds.erase(correspondDscMsg);

    // if now pendingMessages is empty, send back shortest route, or print.
    if(_dscObjs.at(originalMsgId).pendingDscMsgIds.empty())
    {

        // send back Nack, if no routes found
        if(_dscObjs.at(originalMsgId).routes.empty())
        {
            if(originalMsgId == ORIGINAL_STARTED)
            {
                cout << "nack" << endl;
            }
            else
            {
                auto nack = MessageGenerator::generateNackMessage
                        (currentNode.getNextMsgNum(), currentNode.getNodeId(), _dscObjs.at(originalMsgId).senderId, originalMsgId);
                auto socketFd = currentNode.getNeighborSocket(_dscObjs.at(originalMsgId).senderId);
                if (send(socketFd, &nack, sizeof(nack), 0) < 0)
                {
                    throw runtime_error("sending back nack message failed\n");
                }
            }
        }
        // send back route, if there is a valid route
        else
        {
            if(originalMsgId == ORIGINAL_STARTED)
            {
                const auto shortestRoute = _dscObjs.at(originalMsgId).getShortestRoute();
                const auto lastElem = shortestRoute.back();

                cout << currentNode.getNodeId() << ",";
                for(const auto& n: shortestRoute)
                {
                    cout << n;
                    if(n != lastElem)
                    {
                        cout << ",";
                    }
                    else
                    {
                        cout << endl;
                    }
                }
            }
            else
            {
                auto shortestRoute = _dscObjs.at(originalMsgId).getShortestRoute();
                auto route = MessageGenerator::generateRouteMessage
                        (currentNode.getNextMsgNum(), currentNode.getNodeId(), _dscObjs.at(originalMsgId).senderId, originalMsgId, (int)shortestRoute.size(), shortestRoute);
                auto socketFd = currentNode.getNeighborSocket(_dscObjs.at(originalMsgId).senderId);
                if (send(socketFd, &route, sizeof(route), 0) < 0)
                {
                    throw runtime_error("sending back route message failed\n");
                }
            }
        }
        _dscObjs.erase(originalMsgId);
    }

}

// Note: Route message payload: [discover-msg-id][num-of-nodes][nodeId1, nodeId2, ....]
void CommunicationCenter::onRouteMessageReceived(Node& currentNode, Message &msg)
{
    int correspondDscMsg;
    memcpy(&correspondDscMsg, msg.payload, sizeof(int));

    int originalMsgId = -1;
    for(const auto& [key, obj] : _dscObjs)
    {
        if(obj.pendingDscMsgIds.find(correspondDscMsg) != obj.pendingDscMsgIds.end())
        {
            originalMsgId = key;
            break;
        }
    }

    if(originalMsgId == -1)
    {
        throw invalid_argument("no such discover msg which suits the route msg payload\n");
    }

    _dscObjs.at(originalMsgId).pendingDscMsgIds.erase(correspondDscMsg);

    // unpack nodes in route
    std::vector<int> nodesInRoute;
    int membersNum, member;
    memcpy(&membersNum, msg.payload + sizeof(int), sizeof(int));
    // print message to the user
    for(int i = 0; i < membersNum; i++)
    {
        memcpy(&member, msg.payload + sizeof(int) * (2 + i), sizeof(int));
        nodesInRoute.push_back(member);
    }

    _dscObjs.at(originalMsgId).routes.push_back(nodesInRoute);

    // if now pendingMessages is empty, send back shortest route (or print it)
    if(_dscObjs.at(originalMsgId).pendingDscMsgIds.empty())
    {
        if(originalMsgId == ORIGINAL_STARTED)
        {
            const auto shortestRoute = _dscObjs.at(originalMsgId).getShortestRoute();
            const auto lastElem = shortestRoute.back();

            cout << currentNode.getNodeId() << ",";
            for(const auto& n: shortestRoute)
            {
                cout << n;
                if(n != lastElem)
                {
                   cout << ",";
                }
                else
                {
                    cout << endl;
                }
            }
        }
        else
        {
            auto shortestRoute = _dscObjs.at(originalMsgId).getShortestRoute();
            auto route = MessageGenerator::generateRouteMessage
                    (currentNode.getNextMsgNum(), currentNode.getNodeId(), _dscObjs.at(originalMsgId).senderId, originalMsgId, (int)shortestRoute.size(), shortestRoute);
            auto socketFd = currentNode.getNeighborSocket(_dscObjs.at(originalMsgId).senderId);
            if (send(socketFd, &route, sizeof(route), 0) < 0)
            {
                throw runtime_error("sending back route message failed\n");
            }
        }
        _dscObjs.erase(originalMsgId);
    }
}

void CommunicationCenter::onDiscoverMessageReceived(Node& currentNode, Message &msg)
{
    int originalDiscoverMsgId = msg.msgId;

    const auto neighbors = currentNode.getNeighbors();
    int searchNode;
    memcpy(&searchNode, msg.payload, sizeof(int));

    // if a neighbor is the searched node, send back a route
    if(neighbors.find(searchNode) != neighbors.end())
    {
        const std::vector<int> nodesIds = {currentNode.getNodeId(), searchNode};
        // send back Route
        auto route = MessageGenerator::generateRouteMessage
                (currentNode.getNextMsgNum(), currentNode.getNodeId(), msg.sourceId, originalDiscoverMsgId, 2, nodesIds);
        auto socketFd = neighbors.at(msg.sourceId).socket;
        if (send(socketFd, &route, sizeof(route), 0) < 0)
        {
            throw runtime_error("sending back route message failed\n");
        }
    }

    // if no neighbor is the searched node, send discover to all neighbors that
    // DO NOT appear in the discover payload
    else
    {
        std::set<int> alreadyVisitedNodesSet;
        std::vector<int> alreadyVisitedNodesVec;
        int alreadyVisitedNodesNum;
        memcpy(&alreadyVisitedNodesNum, msg.payload + sizeof(int), sizeof(int));

        DiscoverObject dscObject;
        for(int i = 0; i < alreadyVisitedNodesNum; i++)
        {
            int nodeNum;
            memcpy(&nodeNum, msg.payload + sizeof(int) * (2 + i), sizeof(int));
            alreadyVisitedNodesSet.insert(nodeNum);
            alreadyVisitedNodesVec.push_back(nodeNum);
        }

        alreadyVisitedNodesVec.push_back(currentNode.getNodeId());
        int sentNeighbors = 0;
        for(const auto &n : neighbors)
        {
            if (alreadyVisitedNodesSet.find(n.first) == alreadyVisitedNodesSet.end())
            {
                int discoverMsgId = currentNode.getNextMsgNum();
                dscObject.pendingDscMsgIds.insert(discoverMsgId);

                const auto discoverMessage = MessageGenerator::generateDiscoverMessage
                        (discoverMsgId, currentNode.getNodeId(), n.first ,
                         searchNode, (int)alreadyVisitedNodesVec.size(), alreadyVisitedNodesVec);

                if (send(n.second.socket, &discoverMessage, sizeof(discoverMessage), 0) < 0)
                {
                    throw runtime_error("sending discover message to neighbors failed\n");
                }
                sentNeighbors++;
            }
        }
        if(sentNeighbors != 0)
        {
            _dscObjs[msg.msgId] = dscObject;
        }
        else
        {
            auto nack = MessageGenerator::generateNackMessage
                    (currentNode.getNextMsgNum(), currentNode.getNodeId(), msg.sourceId, msg.msgId);
            auto socketFd = currentNode.getNeighborSocket(msg.sourceId);
            if (send(socketFd, &nack, sizeof(nack), 0) < 0)
            {
                throw runtime_error("sending back nack message failed\n");
            }
            return;
        }
    }
    _dscObjs[msg.msgId].senderId = msg.sourceId;
}


void CommunicationCenter::onSendMessageReceived(Node& currentNode, Message &msg)
{
    if(msg.destId == currentNode.getNodeId())
    {
        int msgLength;
        memcpy(&msgLength, msg.payload, sizeof(int));
        // print message to the user
        for(int i = sizeof(int); i < sizeof(int) + msgLength; i++)
        {
            cout << msg.payload[i];
        }
        cout << endl;

        // send Ack message
        auto ackMsg = MessageGenerator::generateAckMessage
                (currentNode.getNextMsgNum(), currentNode.getNodeId(), msg.sourceId, msg.msgId);
        if (send(currentNode.getNeighborSocket(msg.sourceId), &ackMsg, sizeof(ackMsg), 0) < 0)
        {
            cerr << "error: sending ack failed." << endl;
        }
    }
    // if message doesn't intend to this node, send Nack
    else
    {
        auto ackMsg = MessageGenerator::generateNackMessage
                (currentNode.getNextMsgNum(), currentNode.getNodeId(), msg.sourceId, msg.msgId);
        if (send(currentNode.getNeighborSocket(msg.sourceId), &ackMsg, sizeof(ackMsg), 0) < 0)
        {
            cerr << "error: sending nack failed." << endl;
        }
    }
}
