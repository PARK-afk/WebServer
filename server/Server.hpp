#ifndef SERVER_HPP
# define SERVER_HPP
# include <iostream>
# include <vector>
# include <map>
# include <cstdio>
# include <unistd.h>
# include <sys/time.h>
# include <sys/event.h>
# include <sys/types.h>
# include <algorithm>

# include "../socket/socket.hpp"
# include "../Structure/config.hpp"

# include "../utils/utils.hpp"

# define READ 0
# define WRITE 1
class Client {
	public:
	Client() {}
}; //수현이 코드 받기 전 임시.

class Server
{
	private:
		//std::vector<FD> _serverList;
		std::vector<struct kevent> _changeList;
		std::vector<class Socket *> _socketList;
		std::map<FD, class Client *> _clients;
		FD _kq;
	public:
		Server();
		~Server();

		void makeServerSocket(Config &Conf);
		void activateSocket(const Config &Conf);
		void queueInit();
		void changeEvents(std::vector<struct kevent> &changeList, uintptr_t ident, int16_t filter, uint16_t flags, uint32_t fflags, intptr_t data, void *udata);
		int socketFDIndex(FD fd);
		
		void run(const Config &Conf);
		void readEventList(int eventNumber, struct kevent *eventList, const Config &Conf);
		void eventHandling(struct kevent &currEvent, const Config &Conf);
		void execute(Client *paramPtr, const Config &Conf);
		void executeCGI(Client &param);
		void disconnectClient(int fd);

		void Server::addNewClient(FD fd);

};
#endif
