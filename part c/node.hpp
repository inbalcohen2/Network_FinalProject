#ifndef UNTITLED3_NODE_HPP
#define UNTITLED3_NODE_HPP

#include <utility>
#include <map>
#include <set>

#include <netdb.h>
#include <netinet/in.h>
#include <cstdlib>
#include <cstring>
#include <sys/socket.h>
#include <sys/types.h>
#include <cstdio>
#include <unistd.h>
#include <arpa/inet.h>
#include <stdexcept>
#include <ifaddrs.h>
#include <iostream>

#include "messageGenerator.h"
#include "select.hpp"
#include "nodeData.hpp"

using namespace std; // TODO remove

constexpr int MAX_QUEUE_SIZE = 50;
constexpr int TCP_PROTOCOL = 0;
constexpr int INPUT_CHANNEL = 0;

class Node
{
    using NodeId = int;

public:
    explicit Node(int port) noexcept (false);
	void setid(int nodeId) noexcept(false);
	void connectToNode(const std::string& destIpAddress, int destPort) noexcept(false);


	void route(int nodeId)
	{
	    if(_neighbors.empty())
	    {
	        throw std::runtime_error("error: route command requires connection to at least one node\n");
	    }
	    // TODO implement


	}

	void peers();

	int getListeningSocket() const { return _nodeData.socket;}
	int getNodeId() const noexcept(false);
	int getPort() const {return _nodeData.port; }

	void addNeighbor(const NodeData& nodeData)
	{
	    if(_neighbors.find(nodeData.id) != _neighbors.end())
	    {
	        throw std::invalid_argument("neighbor already exist!\n");
	    }
	    _neighbors.insert({nodeData.id, nodeData});
	}

	std::map<NodeId, NodeData> getNeighbors() const
	{
	    return _neighbors;
	}


	int getNeighborSocket(int neighborId) const
	{
	    if(_neighbors.find(neighborId) == _neighbors.end())
	    {
	        throw std::invalid_argument("no neighbor with the given id is exist\n");
	    }
	    return _neighbors.at(neighborId).socket;
	}

	void printNeighbors() const
	{
	    for(const auto& [key, value] : _neighbors)
	    {
	        std::cout << value;
	        std::cout << "--------" << std::endl;
	    }
	}

	static std::string getIpAddress();

	~Node()
	{
	    close(_nodeData.socket);
	}

	int getNextMsgNum() noexcept(false);

private:

    bool _isSetId = false;
    int msgCounter = 0;

    NodeData _nodeData;
	std::map<NodeId, NodeData> _neighbors;

	std::map<NodeId, int> _pendingDiscoverRequest;
};


#endif //UNTITLED3_NODE_HPP
