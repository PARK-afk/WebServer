#ifndef REQUESTHANDLE_HPP
# define REQUESTHANDLE_HPP

# include <iostream>
# include <string>
# include <map>
# include <sstream>
# include <algorithm>
# include <sys/types.h>
# include <sys/stat.h>
# include <cstring>
# include <iomanip>
# include <dirent.h>
# include "structRq.hpp"
# include "../Manager/Manager.hpp"
# include "../utils/utils.hpp"
# include "../Parse/Config.hpp"
# include <sys/time.h>

class Config;
class Client;

enum {
    READ_NOT_DONE = 0,
    READ_LINE_DONE = 1,
    READ_HEADER_DONE = 2,
    READ_BODY_DOING = 3,
    READ_DONE = 4,
    READ_ERROR = 5
};

class RequestHandle {
    private:
        std::string _buffer;
        Request     _request;
        Port         _port;
        int         _readStatus;
        int         _responseStatus;
        bool        _isKeepAlive;
        std::string _tempResult;
		void        parseRequestLine(const std::string& line);
		void        parseHeader(const std::string& line);
        void        parseRegularBody(std::string& body);
		void        validateRequest();
		void        setCookie();
		void        parseChunkedBody(const std::string& body);
        void        parseUri(const std::string& uri);
        std::string parseMethod(const std::string& methodStr);
		std::string parsePart(const std::string& body, const std::string& boundary);
		std::string parseFileContent(const std::string &body);
		std::string parseBodyHeader(const std::string& part);
		std::string parseType(const std::string& body_header);
		std::string parseFileName(const std::string& body_header);
		std::string parseBoundary(const std::string& body_header);
		std::string parseContentType(std::string &body_header);
        RequestHandle();

    public:
        RequestHandle(Port port);
        RequestHandle(const RequestHandle &Copy);
        ~RequestHandle();

		std::string	 			getHost() const;
        Port            		getPort() const;
        std::string     		getBuffer() const;
        std::string     		getMethod() const;
        std::string     		getUri() const;
        std::string     		getVersion() const;
        std::string     		getHeader(const std::string& key) const;
        std::string     		getCookie(const std::string& key) const;
        std::string     		getBody() const;
        bool               		getIsKeepAlive() const;   
        const Request   		&getRequest() const;
        int             		getResponseStatus() const;
        void                    setResponseStatus(int num) {_responseStatus = num;}
        void            		setRequest();
        void            		setBuffer(unsigned char *buffer, int length);
        void            		clearRequest();
        void            		clearAll();
        int             		getReadStatus() const {return this->_readStatus;}
        void                    setReadStatus(int readStatus) {_readStatus = readStatus;}
		const std::string		&getQuery() const;
        


        void printAllHeaders() const;
        std::string getTempResult() const {
            return this->_tempResult;
        }
};

#endif