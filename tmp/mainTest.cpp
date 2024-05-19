#include "../Server/EchoServer.hpp"
#include "../Parse/Config.hpp"
#include "../utils/Error.hpp"
#include <signal.h>

using namespace std;

int main(int ac, char **av, char **env)
{
	// signal(SIGPIPE, SIG_IGN);
	Error::initializeError();
	Config Conf(ac, av);
	Server Server;
	Server.makeServerSocket(Conf);
	Server.queueInit(Conf);
	Server.run(Conf, env);
}
