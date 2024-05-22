#ifndef ECHOSERVER_HPP
# define ECHOSERVER_HPP
# include <iostream>
# include <vector>
# include <map>
# include <cstdio>
# include <unistd.h>
# include <algorithm>
# include <fstream>
# include <sstream>
#include <ctime>


# include <sys/time.h>
# include <sys/event.h>
# include <sys/types.h>

# include "../socket/socket.hpp"
# include "../Parse/Config.hpp"
# include "../utils/utils.hpp"
# include "../request/Client.hpp"
# include "../request/Request.hpp"
# include "../utils/Error.hpp"

# define READ 0
# define WRITE 1

class Server
{
	private:
		std::vector<struct kevent> _changeList;
		std::vector<Socket *> _serverSocketList;
		std::map<FD, Client *> _clientMap;
		std::vector<FD> _closeList;
		FD _kq;
	protected:
        
		void activateSocket(const Config &Conf);
		void eventHandling(struct kevent &currEvent, const Config &Conf);
		void disconnectClient(int fd);
		void addNewClient(FD fd);
		void updateControl();
		int FDIndexing(FD fd);
		void changeEvents(std::vector<struct kevent> &changeList, uintptr_t ident, int16_t filter, uint16_t flags, uint32_t fflags, intptr_t data, void *udata);
		int nonblocking();
		time_t setConnectTime(int sec);
	public:
		Server();
		~Server();
		
		void makeServerSocket(Config &Conf);
		void queueInit(const Config &Conf);
		void run(const Config &Conf);
		void handleClientRead(FD clientFD, const Config &Conf);
		void handleClientWrite(FD clientFD);
		void handleClientCgi(struct kevent &currEvent, const Config &conf);
		void delayResponse(double seconds);
};
#endif
