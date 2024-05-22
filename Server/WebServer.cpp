#include "WebServer.hpp"
#include <sys/time.h>

Server::Server() {}

Server::~Server() {
	for (std::vector<Socket *>::iterator it = _serverSocketList.begin(); it != _serverSocketList.end(); ++it) {
		delete *it;
	}
	for (std::map<FD, Client *>::iterator it = _clientMap.begin(); it != _clientMap.end(); ++it) {
		delete it->second;
	}
}

void Server::makeServerSocket(Config &Conf) {
	for (int i = 0; i < Conf.getNumberOfServer(); i++) {
		Socket *elem = new Socket(Conf[i].getServerName(), Conf[i].getPortName());
		_serverSocketList.push_back(elem);
	}
	activateSocket(Conf);
}

void Server::activateSocket(const Config &Conf) {
	for (int i = 0; i < Conf.getNumberOfServer(); ++i)
		_serverSocketList[i]->autoActivate();
}

int Server::nonblocking() {
    int flags = fcntl(_kq, F_GETFL, 0);

    if (flags == -1) {
        std::cerr << "Error: Failed to get socket flags. Error code: " << errno << std::endl;
        return FAILURE;
    }

    flags |= O_NONBLOCK;
    if (fcntl(_kq, F_SETFL, flags) == -1) {
        std::cerr << "Error: Failed to set socket flags. Error code: " << errno << std::endl;
        return FAILURE;
    }
    return SUCCESS;
}

void Server::queueInit(const Config &Conf) {
	_kq = kqueue();

	for(int i = 0; i < Conf.getNumberOfServer(); ++i) {
		changeEvents(_changeList, static_cast<FD>(*_serverSocketList[i]), EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, nullptr);
	}
}

void Server::changeEvents(std::vector<struct kevent> &changeList, uintptr_t ident, int16_t filter, uint16_t flags, uint32_t fflags, intptr_t data, void *udata) {
	struct kevent tempEvent;

    EV_SET(&tempEvent, ident, filter, flags, fflags, data, udata);
    changeList.push_back(tempEvent);
}

time_t Server::setConnectTime(int sec) {
	struct timespec timeout;

	timeout.tv_sec = sec;
	timeout.tv_nsec = 0;	
	return timeout.tv_sec;
}

int Server::FDIndexing(FD fd) {
	for (size_t i = 0; i < _serverSocketList.size(); ++i) {
		if (static_cast<FD>(*_serverSocketList[i]) == fd)
			return i;
	}
	return -1;
}

void Server::addNewClient(FD fd) {
	int newFD;
	int opt = 1;
	
	newFD = Socket::accept(fd);
	if (newFD == -1) {
		std::cerr << "Error accepting new client" << std::endl;
		return;
	}


	std::cout << "hi new client. | fd : " << newFD << std::endl;
	setsockopt(newFD, SOL_SOCKET, SO_NOSIGPIPE, &opt, sizeof(opt));
	setsockopt(newFD, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
	setsockopt(newFD, SOL_SOCKET, SO_REUSEPORT, &opt, sizeof(opt));
	setsockopt(newFD, SOL_SOCKET, SO_KEEPALIVE, &opt, sizeof(opt));
	Client *Ptr = new Client(_serverSocketList[FDIndexing(fd)]->getPort());
	_clientMap[newFD] = Ptr;
	changeEvents(_changeList, newFD, EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, NULL);
	changeEvents(_changeList, newFD, EVFILT_WRITE, EV_ADD | EV_DISABLE, 0, 0, NULL);
}

void Server::updateControl() {
	_changeList.clear();
}


void Server::disconnectClient(int fd) {
    std::map<FD, Client *>::iterator it = _clientMap.find(fd);
    if (it == _clientMap.end()) {
        std::cout << "Client not found: " << fd << std::endl;
        return;
    }
    delete it->second;
    _clientMap.erase(it);
    changeEvents(_changeList, fd, EVFILT_READ, EV_DELETE, 0, 0, NULL);
    changeEvents(_changeList, fd, EVFILT_WRITE, EV_DELETE, 0, 0, NULL);
    std::cout << "Client disconnected: " << fd << std::endl;
	if (close(fd) == -1) {
        std::cerr << "Error closing socket: " << fd << std::endl;
    }
	delayResponse(0.0002f);

}



void Server::delayResponse(double seconds) {
    clock_t startTime = clock();
    double elapsedSeconds = 0.0;

    while (elapsedSeconds < seconds) {
        clock_t currentTime = clock();
        elapsedSeconds = static_cast<double>(currentTime - startTime) / CLOCKS_PER_SEC;
    }
}


void Server::handleClientRead(FD clientFd, const Config &Conf) {
    unsigned char *buffer = new unsigned char[1025];
    std::memset(buffer, 0, 1025);
    ssize_t length;

    length = recv(clientFd, buffer, 1024, 0);
    
    if (length < 0) {
        std::cerr << "Error reading from client: " << clientFd << std::endl;
        delayResponse(0.0002f);
        delete[] buffer;
        return ;
    } else if (length == 0) {
        std::cout << "Client disconnected: " << clientFd << std::endl;
        disconnectClient(clientFd);
        delete[] buffer;
        return ;
    }
    buffer[length] = '\0';
    Client *ptr = _clientMap[clientFd];
    ptr->setBuffer(buffer, length);
    delete[] buffer;
    
    if (ptr->getReadStatus() == READ_DONE || ptr->getReadStatus() == READ_ERROR) {
            ptr->generateResponse(Conf);
        if (ptr->iscgi()) {
            ptr->getProcInfo()->clientFd = clientFd;
            changeEvents(_changeList, ptr->getProcInfo()->pid, EVFILT_PROC, EV_ADD | EV_ENABLE, NOTE_EXIT | NOTE_EXITSTATUS, 0, ptr->getProcInfo());
            changeEvents(_changeList, clientFd, EVFILT_READ, EV_DISABLE, 0, 0, NULL);
        } else {
            changeEvents(_changeList, clientFd, EVFILT_WRITE, EV_ENABLE, 0, 0, NULL);
            changeEvents(_changeList, clientFd, EVFILT_READ, EV_DISABLE, 0, 0, NULL);
        }
        return ;
    }
}

void Server::handleClientCgi(struct kevent &currEvent, const Config & Conf) {
	procInfo *procPtr = static_cast<procInfo *>(currEvent.udata);
	Client *ptr = _clientMap[procPtr->clientFd];
	int tempFileFd;
	int length = -1;
	char buffer[1025];

	if (currEvent.data == InternalServerError_500) {
		std::remove(procPtr->tempFileNameIn.c_str());
		std::remove(procPtr->tempFileNameOut.c_str());
		ptr->setResponse(Error::errorHandler(Conf[ptr->getPort()], InternalServerError_500));
		waitpid(procPtr->pid, NULL, 0);
		changeEvents(_changeList, procPtr->clientFd, EVFILT_WRITE, EV_ENABLE, 0, 0, NULL);
		changeEvents(_changeList, procPtr->clientFd, EVFILT_READ, EV_DISABLE, 0, 0, NULL);
		return ;
	}
	tempFileFd = open(procPtr->tempFileNameOut.c_str(), O_RDONLY);
    if (tempFileFd == -1) {
        std::remove(procPtr->tempFileNameIn.c_str());
		std::remove(procPtr->tempFileNameOut.c_str());
		ptr->setResponse(Error::errorHandler(Conf[ptr->getPort()], InternalServerError_500));
        waitpid(procPtr->pid, NULL, 0);
        changeEvents(_changeList, procPtr->clientFd, EVFILT_WRITE, EV_ENABLE, 0, 0, NULL);
		changeEvents(_changeList, procPtr->clientFd, EVFILT_READ, EV_DISABLE, 0, 0, NULL);

    }

	while (length) {
		memset(buffer, 0, 1025);
		length = read(tempFileFd, buffer, 1024);
		if (length == -1)
			break;
		ptr->appendResponse(buffer);
	}

	close(tempFileFd);
	std::remove(procPtr->tempFileNameIn.c_str());
	std::remove(procPtr->tempFileNameOut.c_str());
	waitpid(procPtr->pid, NULL, 0);
	if (length == -1) {
        std::cerr << "length == -1" << std::endl;
		changeEvents(_changeList, procPtr->clientFd, EVFILT_WRITE, EV_ENABLE, 0, 0, NULL);
		changeEvents(_changeList, procPtr->clientFd, EVFILT_READ, EV_DISABLE, 0, 0, NULL);
	} else {
		changeEvents(_changeList, procPtr->clientFd, EVFILT_WRITE, EV_ENABLE, 0, 0, NULL);
	}
}

void Server::run(const Config &Conf) {
    int eventNumber;
    struct kevent eventList[1024];
    
    struct timespec timeout;
    timeout.tv_sec = 10;
    timeout.tv_nsec = 0;
    
    while (true)
	{
		eventNumber = kevent(_kq, &_changeList[0], _changeList.size(), eventList, 10, NULL);
        updateControl();
        
        if (eventNumber < 0)
            continue;
        
        for (int i = 0; i < eventNumber; ++i) {
            if (eventList[i].flags & EV_ERROR) {
                std::cout << "Event error occurred" << std::endl;
                break;
            }
            
            if (eventList[i].filter == EVFILT_TIMER) {
                disconnectClient(eventList[i].ident);
            } else if (eventList[i].filter == EVFILT_READ) {
				
                if (FDIndexing(eventList[i].ident) >= 0) {
                    addNewClient(eventList[i].ident);
                } else {
                    handleClientRead(eventList[i].ident, Conf);
                }
            } else if (eventList[i].filter == EVFILT_WRITE) {
				const float fixedDelaySeconds = 0.0018f;
				const clock_t fixedDelayClocks = static_cast<clock_t>(fixedDelaySeconds * CLOCKS_PER_SEC);

				clock_t startTime = clock();
				handleClientWrite(eventList[i].ident);
				clock_t endTime = clock();

				clock_t elapsedClocks = endTime - startTime;
				if (elapsedClocks < fixedDelayClocks) {
					float remainingSeconds = std::round(static_cast<float>(fixedDelayClocks - elapsedClocks) / CLOCKS_PER_SEC * 100000.0f) / 100000.0f;
					delayResponse(remainingSeconds);
				}
			} else if (eventList[i].filter == EVFILT_PROC) {
				handleClientCgi(eventList[i], Conf);
			}
        }
    }
}

void Server::handleClientWrite(FD clientFd) {
    Client *ptr = _clientMap[clientFd];

    ssize_t length = send(clientFd, ptr->getResponse().c_str(), ptr->getResponse().length(), 0);

    
    if (length <= 0) {
        disconnectClient(clientFd);
        return;
    }
    
    ptr->cutResponse(length);
    
    if (ptr->getResponse().length() == 0) {
        if (!ptr->getIsKeepAlive()) {
            disconnectClient(clientFd);
        } else {
            ptr->clearAll();
            changeEvents(_changeList, clientFd, EVFILT_WRITE, EV_DISABLE, 0, 0, NULL);
            changeEvents(_changeList, clientFd, EVFILT_READ, EV_ENABLE, 0, 0, NULL);
        }
    }
}