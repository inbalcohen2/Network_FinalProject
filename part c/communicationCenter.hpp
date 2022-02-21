#ifndef UNTITLED3_COMMUNICATIONCENTER_HPP
#define UNTITLED3_COMMUNICATIONCENTER_HPP

#include <iostream>
#include <map>
#include <algorithm>
#include "node.hpp"

constexpr int ORIGINAL_STARTED = -2;

struct DiscoverObject{
    [[nodiscard]] std::vector<int> getShortestRoute() const;
    int senderId; // who sent this discover message
    std::set<int> pendingDscMsgIds; // discover messages were sent from this node (and he is waiting for route / nack)
    std::vector<std::vector<int>> routes; // routes were founded and returned to this node
};


class CommunicationCenter {
    using OriginalDscId = int;

public:
    CommunicationCenter() = delete;

    static void SendMessage(Node& currentNode, int destNodeId, int msgLength, const std::string& msgToSend);
    static void printRoute(Node& node, int destNodeId);

    static void onReceivedConnectionRequest(Node& currentNode);
    static void onMessageReceived(Node& currentNode, Message &msg);

private:
    static void onSendMessageReceived(Node& currentNode, Message &msg);
    static void onNackMessageReceived(Node& currentNode, Message &msg);
    static void onRouteMessageReceived(Node& currentNode, Message &msg);
    static void onDiscoverMessageReceived(Node& currentNode, Message &msg);

    static std::map<OriginalDscId, DiscoverObject> _dscObjs;
};

inline std::map<int, DiscoverObject> CommunicationCenter::_dscObjs = {};

inline std::vector<int> DiscoverObject::getShortestRoute() const
{
    if (routes.empty())
        return {};

    size_t minSize = routes[0].size();
    for(const auto &r : routes)
    {
        if(r.size() < minSize)
            minSize = r.size();
    }

    std::vector<std::vector<int>> shortestRoutes = {};
    for(const auto &r : routes)
    {
        if(r.size() == minSize)
            shortestRoutes.push_back(r);
    }

    std::sort(shortestRoutes.begin(), shortestRoutes.end());

    return shortestRoutes[0];
}



#endif //UNTITLED3_COMMUNICATIONCENTER_HPP
