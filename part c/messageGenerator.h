#ifndef UNTITLED3_MESSAGEGENERATOR_H
#define UNTITLED3_MESSAGEGENERATOR_H

#include <cstdio>
#include <vector>
#include "message.hpp"

class MessageGenerator
{
public:
    MessageGenerator() = delete;
//    int msgId;
//    int sourceId;
//    int destId;
//    int trailingMsg;
//    funcId func;
//    char payload[PAYLOAD_SIZE];

    static Message generateAckMessage(int msgNum, int sourceId, int destId, int msgIdToBeAcked)
    {
        auto m = generateBasicMessage(msgNum, sourceId);
        m.destId = destId;
        m.func = Ack;
        memcpy(m.payload, (char *)&msgIdToBeAcked, sizeof(int));
        return m;
    }

    static Message generateNackMessage(int msgNum, int sourceId, int destId, int msgIdToBeNacked)
    {
        auto m = generateBasicMessage(msgNum, sourceId);
        m.destId = destId;
        m.func = Nack;
        memcpy(m.payload, (char *)&msgIdToBeNacked, sizeof(int));
        return m;
    }

    static Message generateConnectMessage(int msgNum, int sourceId)
    {
        auto m = generateBasicMessage(msgNum, sourceId);
        m.func = Connect;
        return m;
    }

    static Message generateDiscoverMessage(int msgNum, int sourceId, int destId, int searchedNodeId,
                                           int numOfVisitedNodes, const std::vector<int>& nodeIds)
    {
        auto m = generateBasicMessage(msgNum, sourceId);
        m.func = Discover;
        m.destId = destId;

        memcpy(m.payload, (char *)&searchedNodeId, sizeof(int));
        memcpy(m.payload + sizeof(int), (char *)&numOfVisitedNodes, sizeof(int));
        for(int i = 0; i < numOfVisitedNodes; i++)
        {
            memcpy(m.payload + sizeof(int) * (i + 2), (char *)&nodeIds[i], sizeof(int));
        }

        return m;
    }

    static Message generateRouteMessage(int msgNum, int sourceId, int destId, int discoverMsgId, int answerLength, const std::vector<int>& nodeIds)
    {
        auto m = generateBasicMessage(msgNum, sourceId);
        m.func = Route;
        m.destId = destId;
        memcpy(m.payload, (char *)&discoverMsgId, sizeof(int));
        memcpy(m.payload + sizeof(int), (char *)&answerLength, sizeof(int));

        for(int i = 0; i < answerLength; i++)
        {
            memcpy(m.payload + sizeof(int) * (i + 2), (char *)&nodeIds[i], sizeof(int));
        }
        return m;
    }

    static Message generateSendMessage(int msgNum, int sourceId, int destId, int msgLength, const std::string& msg)
    {
        auto m = generateBasicMessage(msgNum, sourceId);
        m.func = Send;
        m.destId = destId;
        memcpy(m.payload, (char *)&msgLength, sizeof(int));
        sprintf(m.payload + sizeof(int), "%s", msg.c_str());
        return m;
    }

//    static Message generateRelayMessage(int msgNum, int sourceId, int destId)
//    {
//        auto m = generateBasicMessage(msgNum, sourceId);
//        m.func = Relay;
//
//        return m;
//    }

private:
    static Message generateBasicMessage(int msgNum, int sourceId)
    {
        Message m = {};
        m.msgId = msgNum;
        m.sourceId = sourceId;
        return m;
    }
};

#endif //UNTITLED3_MESSAGEGENERATOR_H
