#ifndef UNTITLED3_NODEDATA_HPP
#define UNTITLED3_NODEDATA_HPP

struct NodeData{
    explicit NodeData(int p_port)
    : port(p_port)
    {}

    NodeData(int p_port, int p_socket, int p_id, std::string p_ip):
    port(p_port),
    socket(p_socket),
    id(p_id),
    ip(std::move(p_ip))
    {}

    friend std::ostream& operator<<(std::ostream& os, const NodeData& nd);

    int port;
    int socket = 0;
    int id = 0;
    std::string ip;
};

inline std::ostream& operator<<(std::ostream& os, const NodeData& nd)
{
    os << "id: " << nd.id << std::endl;
    os << "ip: " << nd.ip << std::endl;
    os << "port: " << nd.port << std::endl;
    os << "socket: " << nd.socket << std::endl;
    return os;
}

#endif //UNTITLED3_NODEDATA_HPP
