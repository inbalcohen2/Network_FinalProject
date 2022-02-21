#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include "select.hpp"
#include <arpa/inet.h>
#include <unistd.h>

#include <iostream>
#include <vector>
#include <algorithm>
#include "communicationCenter.hpp"

using namespace std;

constexpr int MESSAGE_BUFF_SIZE = 512;
constexpr int ARGS_NUM = 1;

void parseAndActivateCommand(Node& node, const std::string& cmd);
std::vector<std::string> splitByDelimiter(std::string cmd, const std::string& del);

int main(int argc, char *argv[])
{
    if(argc != ARGS_NUM + 1)
    {
        throw std::invalid_argument("usage: port number for the new node\n");
    }

    int port = stoi(argv[1]);
    Node node(port);

    while (true)
    {
        char buff[MESSAGE_BUFF_SIZE] = {0};
        printf("waiting for input...\n");
        int fd = wait_for_input();

        if(fd == INPUT_CHANNEL)
        {
            read(fd, buff, MESSAGE_BUFF_SIZE);
            parseAndActivateCommand(node, buff);
        }
        // if someone tries to connect
        else if(fd == node.getListeningSocket())
        {
            CommunicationCenter::onReceivedConnectionRequest(node);
        }
        // already-connected node sends a message
        else
        {
            Message m = {};
            if (recv(fd, &m, sizeof(m), 0) == -1)
            {
                cerr << "error: receive failed." << endl;
            }
            CommunicationCenter::onMessageReceived(node, m);
        }
    }
}

void parseAndActivateCommand(Node& node, const std::string& cmd)
{
    const auto words = splitByDelimiter(cmd, ",");
    const auto command = words[0];
    if(command == "setid")
    {
        node.setid(stoi(words[1]));
        printf("ack\n");
    }
    else if (command == "connect")
    {
        const auto numStrs = splitByDelimiter(words[1], ":");
        node.connectToNode(numStrs[0], stoi(numStrs[1]));
        //node.connectToNode("127.0.0.1", stoi(numStrs[0]));
    }
    else if (command == "send")
    {
        CommunicationCenter::SendMessage(node, stoi(words[1]), stoi(words[2]), words[3]);
    }
    else if (command == "route")
    {
        CommunicationCenter::printRoute(node, stoi(words[1]));
    }
    else if (command == "peers")
    {
        node.peers();
    }
    else
    {
        throw std::invalid_argument("command is not supported\n");
    }
}

std::vector<std::string> splitByDelimiter(std::string cmd, const std::string& del)
{
    std::vector<std::string> vecToReturn;
    size_t pos;
    while ((pos = cmd.find(del)) != std::string::npos) {
        vecToReturn.push_back(cmd.substr(0, pos));
        cmd.erase(0, pos + del.length());
    }
    vecToReturn.push_back(cmd);

    // trim command from right
    if(vecToReturn.size() == 1)
    {
        vecToReturn[0].erase(std::find_if(vecToReturn[0].rbegin(), vecToReturn[0].rend(), [](unsigned char ch) {
            return !std::isspace(ch);
        }).base(), vecToReturn[0].end());
    }

    return vecToReturn;
}