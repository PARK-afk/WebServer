#ifndef CLIENT_HPP
#define CLIENT_HPP
#include <iostream>
#include <unistd.h>
#include "RequestHandle.hpp"
#include "ResponseHandle.hpp"
#include "../utils/Error.hpp"
#include "../utils/utils.hpp"
#include "../Manager/Manager.hpp"
#include "../socket/socket.hpp"
#include "../Manager/Manager.hpp"
#include <cstdlib>

#define READ 0
#define WRITE 1

typedef struct s_procInfo {
    pid_t pid;
	FD clientFd;
	std::string tempFileNameIn;
	std::string tempFileNameOut;
} procInfo;

class Client {
	private:
		Port 			_port;
		RequestHandle	_requestHandle;
		ResponseHandle	_responseHandle;
		std::string		_response;
		procInfo 		*_procPtr;
		
		Client();
	public:
		Client(Port port);
		Client(const Client &Copy);
		~Client();
		void		clearAll();
		void		setBuffer(unsigned char *buffer, int length);
		int 		getReadStatus() const;
		bool		getIsKeepAlive() const;
		void		generateResponse(const Config &Conf);
		void		handleCGI();
		void		setEnv(const RequestHandle &Req);
		void		setBufferFromChild(int data);
		procInfo*	getProcInfo() const;
		void		makeExecuteCommand(std::string &extention);
		void		setEnv(const Config &Conf, const RequestHandle &Req);
		void		makeTempFileNameForCgi(std::string &filePath, int mode);
		void		handleCGI(const Config &Conf);

		const ResponseHandle& getResponseHandle() const;

		Port		getPort() const;
		std::string getResponse() const;
		void		setResponse(const std::string &param);
		void		appendResponse(const char *param);
		void		cutResponse(int length);
		bool		iscgi() const {
			return _responseHandle.isCGI();
		}
};
#endif