#include "socket.hpp"

void Socket::__init__(std::string host, Port port) {
    struct addrinfo hints;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;

    std::string port_str = std::to_string(port);

    int status = getaddrinfo(host.c_str(), port_str.c_str(), &hints, &_addrInfo);
    if (status != 0) {
        std::cerr << "Error: Failed to get address info. Error code: " << status << std::endl;
        throw std::runtime_error("Error: Failed to get address info");
    }
}

int Socket::getSocketOPT(const FD &socket) {
	int flags;
	socklen_t optlen = sizeof(flags);

	if (::getsockopt(socket, SOL_SOCKET, SO_REUSEADDR, &flags, &optlen) == FAILURE) {
		std::cerr << "Error: Failed to get socket option. Error code: " << errno << std::endl;
		throw std::runtime_error("Error: Failed to get socket option");
	}
	return flags;
}

void Socket::__init__(Port port) {
    struct addrinfo hints;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    std::string port_str = std::to_string(port);

    int status = getaddrinfo(NULL, port_str.c_str(), &hints, &_addrInfo);
    if (status != 0) {
        std::cerr << "Error: Failed to get address info. Error code: " << status << std::endl;
        throw std::runtime_error("Error: Failed to get address info");
    }
}

Socket::Socket(std::string host, Port port) : _listenSocket(-1), _port(port), _host(host) {
    if (host.empty()) {
        std::cout << "Host is empty" << std::endl;
        __init__(port);
    } else {
        host = "localhost";
        __init__(host, port);
    }
    std::cout << "Socket initialized with host and port" << std::endl;
}

Socket::Socket(const Socket& other) : _port(other._port), _host(other._host), _addrInfo(other._addrInfo) {
    if (other._listenSocket != -1) {
        _listenSocket = dup(other._listenSocket);
    }
    std::cout << "Socket copied" << std::endl;
}

FD Socket::socket(int domain, int type, int protocol) {
    std::cout << "Socket created" << std::endl;
    _listenSocket = ::socket(domain, type, protocol);
    if (_listenSocket == -1) {
        std::cerr << "Error: Socket creation failed. Error code: " << errno << std::endl;
        throw std::runtime_error("Error: Socket creation failed");
    }
	try {
		getProtocolName();
	} catch (std::runtime_error &e) {
		std::cerr << "Error: is not TCP protocol" << std::endl;
		throw std::runtime_error("Error: is not TCP protocol");
	}
    return _listenSocket;
}

Status Socket::bind() {
    if (::bind(_listenSocket, _addrInfo->ai_addr, _addrInfo->ai_addrlen) == FAILURE) {
        std::cerr << "Error: Failed to bind socket. Error code: " << errno << std::endl;
        throw std::runtime_error("Error: Failed to bind socket");
    }
    return SUCCESS;
}

Status Socket::listen(size_t backlog) {
    if (::listen(_listenSocket, backlog) == FAILURE) {
        std::cerr << "Error: Failed to listen socket. Error code: " << errno << std::endl;
        throw std::runtime_error("Error: Failed to listen socket");
    }
    return SUCCESS;
}

FD Socket::accept(FD _listenSocket) {
    FD Client_Socket = -1;

    Client_Socket = ::accept(_listenSocket, NULL, NULL);
    if (Client_Socket == -1) {
        if (errno == EAGAIN || errno == EWOULDBLOCK) {
            return Client_Socket;
        } else {
            std::cerr << "Error: Failed to accept socket. Error code: " << errno << std::endl;
            throw std::runtime_error("Error: Failed to accept socket");
        }
    }

    return Client_Socket;
}

Status Socket::close() {
    if (_listenSocket == -1) {
        std::cout << "Socket already closed" << std::endl;
        return SUCCESS;
    }

    if (::close(_listenSocket) == FAILURE) {
        std::cerr << "Error: Failed to close socket. Error code: " << errno << std::endl;
        throw std::runtime_error("Error: Failed to close socket");
    }
    _listenSocket = -1;
    return SUCCESS;
}

Status Socket::nonblocking() {
    int flags = fcntl(_listenSocket, F_GETFL, 0);

    if (fcntl(_listenSocket, F_SETFL, flags | O_NONBLOCK) == -1) {
        std::cerr << "Error: Failed to set socket flags. Error code: " << errno << std::endl;
        return FAILURE;
    }
    return SUCCESS;
}

Status Socket::nonblocking(const FD &socket) {
    int flags = fcntl(socket, F_GETFL, 0);

    if (fcntl(socket, F_SETFL, flags | O_NONBLOCK) == -1) {
        std::cerr << "Error: Failed to set socket flags. Error code: " << errno << std::endl;
        return FAILURE;
    }
    return SUCCESS;
}

void Socket::setSocketOption(int level, int option_name, int opt) {
    socklen_t optlen = sizeof(opt);

    if (setsockopt(this->_listenSocket, level, option_name, &opt, optlen) == FAILURE) {
        std::cerr << "Error: Failed to set socket option. Error code: " << errno << std::endl;
        throw std::runtime_error("Error: Failed to set socket option");
    }
}

int setReceiveBufferSize(FD _fd, int size) {
	if (setsockopt(_fd, SOL_SOCKET, SO_RCVBUF, &size, sizeof(size)) == -1) {
		std::cerr << "Error setting socket receive buffer size: " << strerror(errno) << std::endl;
		return -1;
	}
	return 0;
}

void Socket::__init__SocketoptAuto(int opt) {
    setSocketOption(SOL_SOCKET, SO_REUSEADDR, opt);
    setSocketOption(SOL_SOCKET, SO_KEEPALIVE, opt);
	setReceiveBufferSize(_listenSocket, 1024);
}

void Socket::autoActivate(int domain, int type, int protocol) {
    socket(domain, type, protocol);
    std::cout << "Socket activated" << std::endl;
    __init__SocketoptAuto(1);
    std::cout << "Socket options set" << std::endl;
    bind();
    std::cout << "Socket bound" << std::endl;
    nonblocking();
    std::cout << "Socket non-blocking set" << std::endl;
    listen(1024);

    std::cout << "Socket listening" << std::endl;
	std::cout << "Host: " << _host << "Host IP: " << getSocketIP() << "Port: " << getPort() << "Protocol: " << getProtocolName() << std::endl;
}

Socket *Socket::clone() const {
    return new Socket(*this);
}

Socket::~Socket() {
    if (_listenSocket == -1) {
        std::cout << "Socket already closed" << std::endl;
    } else {
        int status = ::close(_listenSocket);
        if (status == -1) {
            std::cerr << "Error: Socket close failed. Error code: " << errno << std::endl;
        } else {
            std::cout << "Socket closed" << std::endl;
        }
    }

    if (_addrInfo != NULL) {
        freeaddrinfo(_addrInfo);
    }
}

Socket& Socket::operator=(const Socket& other) {
    if (this != &other) {
        if (::close(_listenSocket) == -1) {
            std::cerr << "Error: Failed to close socket before assignment. Error code: " << errno << std::endl;
        }
        this->_listenSocket = dup(other._listenSocket);
        if (this->_listenSocket == -1) {
            std::cerr << "Error: Failed to duplicate socket during assignment. Error code: " << errno << std::endl;
            throw std::runtime_error("Error: Failed to duplicate socket during assignment");
        }
        this->_addrInfo = other._addrInfo;
    }
    return *this;
}

void Socket::setAddrInfo(struct addrinfo *addrInfo) {
    _addrInfo = addrInfo;
}

std::string Socket::getSocketIP() const {
    char ipstr[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &((struct sockaddr_in *)_addrInfo->ai_addr)->sin_addr, ipstr, INET_ADDRSTRLEN);
    return std::string(ipstr);
}

Port Socket::getSocketPort() const {
    return ntohs(((struct sockaddr_in *)_addrInfo->ai_addr)->sin_port);
}

std::string Socket::getProtocolName() const {
    struct protoent *proto = getprotobyname("tcp");
    if (proto == NULL) {
        std::cerr << "Error: Failed to get protocol name. Error code: " << errno << std::endl;
        throw std::runtime_error("Error: Failed to get protocol name");
    }
    return std::string(proto->p_name);
}