#pragma once

#include "RequestHandle.hpp"
#include "../Parse/Config.hpp"
#include "../utils/utils.hpp"
#include "Request.hpp"
#include "Response.hpp"
#include "../utils/Error.hpp"
#include <iostream>
#include <regex>
#include <iomanip>
#include <functional>
#include <fstream>
#include <sstream>



class Response;

class ResponseHandle {
	private:
		bool			_isInitFromLocation;
		std::string 	_response;
		std::string 	_filePath;
		std::string 	_pathInfo;
		std::string 	_scriptName;
		std::string 	_httpUri;
		std::string 	_serverRoot;
		Port			_port;
		LocationConfig	_loc;
	public:
		bool 			initPathFromLocation(const RequestHandle &Req, const Config &Conf);
		ResponseHandle();
		ResponseHandle(const ResponseHandle &Copy);
		~ResponseHandle();
		std::string	generateHTTPFullString(const RequestHandle &Req, const Config &Conf);
		std::string	handleGetRequest(const RequestHandle &Req, const Config &Conf);
		std::string handlePostRequest(const RequestHandle &Req);
		Response	handleMethodNotAllowed();
		std::string getFilePath(const std::string &serverRoot, const std::string &httpUri, LocationConfig &loc);
		Response 	handleRedirect(const LocationConfig &location);
		void		handleAutoIndex(Response &response, const std::string &servRoot);
		std::string	handleDeleteRequest(const Config &Conf);
		void		setResponse(const std::string &response);
		void		clearAll();
		const std::string getResponse();
		bool		isCGI() const;
		std::string	getFilePath() const;
		std::string	getPathInfo() const;
		std::string	getScriptName() const;
		std::string	getHttpUri() const;
		std::string	getServerRoot() const;
		Port		getPort() const;
		const LocationConfig&	getLocation() const;
		ResponseHandle& operator=(const ResponseHandle &Copy);
};

