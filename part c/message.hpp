#ifndef UNTITLED3_MESSAGE_HPP
#define UNTITLED3_MESSAGE_HPP

constexpr int PAYLOAD_SIZE = 492;

enum funcId{
	Ack = 1, Nack = 2, Connect = 4,
	Discover = 8, Route = 16, Send = 32,
	Relay = 64 };

struct Message
{
	int msgId;
	int sourceId;
	int destId;
	int trailingMsg;
	funcId func;
	char payload[PAYLOAD_SIZE];
};

// inline int Message::msgSerialNum = 0;


#endif
