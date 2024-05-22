#include "ResponseHandle.hpp"
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>
#include "NResponseUtils.hpp"
#include "../Manager/Manager.hpp"

ResponseHandle::ResponseHandle() : _isInitFromLocation(false) {
}

ResponseHandle::ResponseHandle(const ResponseHandle &Copy) : _response(Copy._response) {}

ResponseHandle::~ResponseHandle() {}


bool ResponseHandle::isCGI() const {
	return _loc.isCgi();
}

void	ResponseHandle::clearAll() {
	_response.clear();
	_filePath.clear();
	_pathInfo.clear();
	_scriptName.clear();
	_httpUri.clear();
}

std::string ResponseHandle::handleDeleteRequest(const Config &Conf) {
    Response tmpResponse;
    std::string response;

	// Response redirectResponse = handleRedirect(_loc);
	// if (redirectResponse.getStatusCode() != 300) {
	// 	return redirectResponse;
	// } //이 부분은 redirect 구현 잘 끝나면 거기에서 에러코드 등 핸들링 추가로 합시다.

    // 파일 읽기
    std::ifstream file(_filePath.c_str(), std::ios::binary);
    if (file.is_open() && file.good()) {
        // 파일 크기 확인
        std::streamsize fileSize = Manager::responseUtils.getFileSize(file);

        // 파일 크기 제한 설정
        const std::streamsize maxFileSize = Conf[_port].getClientMaxBodySize();
        if (fileSize > maxFileSize) {
			throw PayloadTooLarge_413;
        }
        std::remove(_filePath.c_str());
        file.close();

        tmpResponse.setStatusCode(204);
    } else {
        if (Manager::responseUtils.isDirectory(_filePath)) {
			if (_loc.getAutoindex() == true) {
            	handleAutoIndex(tmpResponse, _filePath);
			} else {
				throw Forbidden_403;
			}
        } else {
			if (_loc.getAutoindex() == false) {
                throw NotFound_404;
			} else {
				handleAutoIndex(tmpResponse, _filePath.substr(0, _filePath.find_last_of('/')));
			}
		}
    }
    tmpResponse.setHeader("Server", "42Webserv");
    response = tmpResponse.getResponses();
    return response;
}

std::string ResponseHandle::generateHTTPFullString(const RequestHandle &Req, const Config &Conf)
{
		initPathFromLocation(Req, Conf);
	int method = Manager::responseUtils.getMethodNumber(Req.getMethod());
	if (Manager::responseUtils.isMethodPossible(method, _loc) == false) {
		throw MethodNotAllowed_405;
	}
	switch (method)
	{
	case GET:
		_response = handleGetRequest(Req, Conf);
		return _response;
	case POST:
		_response = handlePostRequest(Req);
		break;
	case DELETE:
		_response = handleDeleteRequest(Conf);
		break;
	default:
		return handleMethodNotAllowed().getResponses();
		break;
	}
	return "";
}

const std::string ResponseHandle::getResponse()
{
	if (_response.empty())
	{
		throw std::invalid_argument("Response is empty");
	}
	return _response;
}

void ResponseHandle::setResponse(const std::string &response)
{
	_response = response;
}

Response ResponseHandle::handleMethodNotAllowed()
{
	Response response;
	response.setStatusCode(MethodNotAllowed_405);
	response.setHeader("Content-Type", "text/html");
	response.setHeader("Date", Manager::responseUtils.getCurTime());

	std::string errorBody = "<html><body><h1>405 Method Not Allowed</h1><p>The requested method is not allowed.</p></body></html>";
	response.setBody(errorBody);
	response.setHeader("Content-Length", Manager::utils.toString(errorBody.length()));
	response.setHeader("Connection", "close");

	return response;
}





std::string ResponseHandle::getFilePath(const std::string &serverRoot, const std::string &httpUri, LocationConfig &loc) {
    std::string filePath;
    std::string alias = loc.getAlias();


    if (!alias.empty() && httpUri.find(alias) == 0) {
        filePath = alias + httpUri.substr(alias.length());
    }

	if (loc.isCgi()) {
		if (httpUri.substr(0, httpUri.find_last_of('/')).find('.') != std::string::npos) {
			_scriptName = httpUri.substr(0, httpUri.find_last_of('/'));
			_pathInfo = httpUri.substr(httpUri.find_last_of('/'));
			_httpUri = _scriptName;
		} else {
			_scriptName = httpUri;
			_pathInfo = "";
			_httpUri = _scriptName;
		}
	}


	if (httpUri.find(loc.getPath()) == std::string::npos || loc.getPath().find('.') != std::string::npos) {
		if (loc.isCgi() == true) {
			filePath = serverRoot + loc.getFastcgiPass() + httpUri.substr(httpUri.find_last_of('/'));
		} else if (loc.getPath().find('.') != std::string::npos) {
			filePath = serverRoot + loc.getRoot() + httpUri.substr(httpUri.find_last_of('/'));
		}else {
			filePath = serverRoot + loc.getRoot() + httpUri;
		}
	} else {
		if (loc.isCgi() == true) {
			filePath = serverRoot + loc.getFastcgiPass() + httpUri.substr(loc.getPath().length(), httpUri.length() - loc.getPath().length());
		} else {
			std::cout << httpUri.substr(loc.getPath().length(), httpUri.length() - loc.getPath().length()) << std::endl;
			filePath = serverRoot + loc.getRoot() + "/" + httpUri.substr(loc.getPath().length(), httpUri.length() - loc.getPath().length());
		}
	}
	filePath = Manager::responseUtils.normalizePath(filePath);
	if (Manager::responseUtils.isDirectory(filePath) && filePath[filePath.length() - 1] != '/') {
		filePath += "/";
	}
    return filePath;
}



Response ResponseHandle::handleRedirect(const LocationConfig &location)
{
	Response response;
	std::string returnCode = location.getReturnCode();
	std::string returnUrl = location.getReturnUrl();

	if (!returnCode.empty() && !returnUrl.empty())
	{
		int statusCode = std::stoi(returnCode);
		response.setRedirect(returnUrl, statusCode);
		
		response.setHeader("Cache-Control", "no-cache, no-store, must-revalidate");
		response.setHeader("Pragma", "no-cache");
		response.setHeader("Expires", "0");

		response.setHeader("Connection", "close");
		response.setBody("42Webserv Redirected");
		return response;
	}
	response.setStatusCode(OK_200);
	return response;
}

bool	ResponseHandle::initPathFromLocation(const RequestHandle &Req, const Config &Conf) {
	_isInitFromLocation = true;
	_httpUri = Req.getUri();
	_port = Req.getPort();

	_httpUri = Manager::responseUtils.nomralizeUrl(_httpUri);
	_serverRoot = Manager::responseUtils.normalizePath(Conf.getServerConfig(_port, Req.getHost()).getPath());
	if (_serverRoot.empty())
	{
		throw InternalServerError_500;
	}
	_loc = Conf.getServerConfig(_port, Req.getHost()).getLocation(_httpUri);
	_filePath = getFilePath(_serverRoot, _httpUri, _loc);
	if (!Manager::responseUtils.isValidPath(_filePath)) {
		throw BadRequest_400;
	}
	if (_filePath.length() > 1 && _filePath[_filePath.length() - 1] == '/') {
		if (Manager::responseUtils.isDirectory(_filePath) == true) {
			std::string tmpPath = _filePath + _loc.getIndex();
			if (Manager::responseUtils.isExist(_filePath) == true) {
				_filePath = tmpPath;
			}
		} else {
			throw NotFound_404;
		}
	} else {
		if (Manager::responseUtils.isExist(_filePath) == false) {
			_filePath = _filePath.substr(0, _filePath.find_last_of('/') + 1);
		}
	}
	return true;
}



std::string ResponseHandle::handleGetRequest(const RequestHandle &Req, const Config &Conf)
{
	Response response;
	if (_loc.isEtag() == true && (!Req.getHeader("If-Modified-Since").empty() || !Req.getHeader("If-None-Match").empty()) &&  \
		 (Req.getHeader("If-Modified-Since") == Manager::responseUtils.lastModify(_filePath) || Req.getHeader("If-None-Match") == Manager::responseUtils.etag(_filePath)))
	{
		std::cerr << "Not Modified" << std::endl;
		response.setStatusCode(NotModified_304);
		response.setHeader("Content-Length", "0");
		response.setHeader("Date", Manager::responseUtils.getCurTime());
		response.setHeader("Server", "42Webserv");
		response.setHeader("Connection", "close");

		return response.getResponses();
	}
	if (_loc.isCgi() == true)
	{
		(void)Req;
	}

	Response redirectResponse = handleRedirect(_loc);
	if (redirectResponse.getStatusCode() != OK_200) {
		return redirectResponse.getResponses();
	}
	std::string extension = Manager::responseUtils.getFileExtension(_filePath);
	std::ifstream file(_filePath.c_str(), std::ios::binary);

    if (file.is_open() && file.good() && Manager::responseUtils.isDirectory(_filePath) == false) {
        std::streamsize fileSize = Manager::responseUtils.getFileSize(file);

		const std::streamsize maxFileSize = Conf[_port].getClientMaxBodySize();
		(void)Conf;
		if (fileSize > maxFileSize)
		{
			throw PayloadTooLarge_413;
		}
		std::string body = Manager::responseUtils.readFileContent(file, fileSize);
		file.close();
		
		if (body.length() != 0) {
			response.setStatusCode(OK_200);
			response.setHeader("Date", Manager::responseUtils.getCurTime());
			response.setHeader("Content-Type", Manager::responseUtils.getContentType(extension));
			response.setBody(body);
			response.setHeader("Content-Length", Manager::utils.toString(body.length()));
		}else {
			response.setStatusCode(NoContent_204);
			response.setHeader("Date", Manager::responseUtils.getCurTime());
			response.setHeader("Content-Type", Manager::responseUtils.getContentType(extension));
		}
		if (_loc.isEtag() == true)
		{
			std::cerr << "ETAG" << std::endl;
			response.setHeader("Last-Modified", Manager::responseUtils.lastModify(_filePath));
			response.setHeader("ETag", Manager::responseUtils.etag(_filePath));
			response.setHeader("Cache-Control", "max-age=3600, public, must-revalidate, no-cache");
			response.setHeader("Expires", Manager::responseUtils.getExpirationTime(3600));
		} else {
			response.setHeader("Cache-Control", "no-cache, no-store, must-revalidate");
			response.setHeader("Pragma", "no-cache");
			response.setHeader("Expires", "0");
		}

	}
	else
	{
		if (Manager::responseUtils.isDirectory(_filePath))
		{
			if (_loc.getAutoindex() == true)
			{
				handleAutoIndex(response, _filePath);
			}
			else
			{
				throw Forbidden_403;
			}
		}
		else
		{
			if (_loc.getAutoindex() == false)
			{
				throw NotFound_404;
			}
			else
			{
				handleAutoIndex(response, _filePath.substr(0, _filePath.find_last_of('/')));
			}
		}
	}
	response.setHeader("Server", "42Webserv");
	return response.getResponses();
}

std::string ResponseHandle::handlePostRequest(const RequestHandle &Req)
{

	std::string responseData;

	std::string contentType = Req.getHeader("Content-Type");
	if ((contentType.find("multipart/form-data") != std::string::npos && contentType.find("boundary") != std::string::npos) \
		|| contentType.find("application/x-www-form-urlencoded") != std::string::npos)
	{
		if (!Req.getBody().empty())
		{
			if (responseData.empty())
				throw InternalServerError_500;
		}
		else {
			throw BadRequest_400;
		}
	}
	else
		throw InternalServerError_500;
	return responseData;
}

void ResponseHandle::handleAutoIndex(Response &response, const std::string &servRoot)
{
	std::string dirPath = servRoot;

	struct stat fileStat;
	std::stringstream body;
	body << "<html>\n<head>\n<title> AutoIndex </title>\n";
	body << "<style>\n"
		<< "th, td {\n"
		<< "    padding-left: 10px;\n"
		<< "    padding-right: 50px;\n"
		<< "}\n"
		<< "</style>\n"
		<< "</head>\n<body>\n";
	body << "<h1>Index of / </h1>\n";
	body << "<hr> <pre>\n<table>\n<tr><th></th><th></th><th></th></tr>\n";

	DIR *dir = opendir(dirPath.c_str());
	if (dir == NULL && errno == EACCES)
		throw Forbidden_403;
	else if (dir == NULL)
		throw NotFound_404;
	if (dir)
	{
		std::vector<std::string> fileList;
		struct dirent *ent;
		while ((ent = readdir(dir)) != NULL)
			fileList.push_back(ent->d_name);
		closedir(dir);

		std::sort(fileList.begin(), fileList.end());

		int count = fileList.size();
		for (int i = 0; i < count; i++)
		{
			std::string fileName = fileList[i];
			if (fileName == ".")
				continue;
			std::string filePath = dirPath + "/" + fileName;
			if (stat(filePath.c_str(), &fileStat) == -1)
				throw ServiceUnavailable_503;
			if (stat(filePath.c_str(), &fileStat) == 0)
			{
				body << "<tr>" << "<td>";
				if (S_ISDIR(fileStat.st_mode))
					body << "<a href=\"" << fileName << "/\">" << fileName + "/" << "</a>";
				else
					body << fileName;
				body << "</td><td>\t\t" << Manager::responseUtils.getFormattedTime(fileStat.st_mtime) << "</td>";
				double fileSize = static_cast<double>(fileStat.st_size);
				body << "<td>\t\t" << Manager::responseUtils.getFormatSize(fileSize) << "</td>" << "</tr>\n";
			}
			else
				throw InternalServerError_500;
		}
		body << " </table> </pre>\n<hr>\n</body>\n</html>\n";
		response.setStatusCode(OK_200);
		response.setHeader("Date", Manager::responseUtils.getCurTime());
		response.setHeader("Content-Type", "text/html");
		response.setBody(body.str());
		response.setHeader("Content-Length", Manager::utils.toString(body.str().length()));
		response.setHeader("Connection", "keep-alive");
	}
}

void printAllEnv()
{
	std::cout << "REQUEST_METHOD: " << getenv("REQUEST_METHOD") << std::endl;
	std::cout << "REQUEST_URI: " << getenv("REQUEST_URI") << std::endl;
	std::cout << "QUERY_STRING: " << getenv("QUERY_STRING") << std::endl;
	std::cout << "SCRIPT_NAME: " << getenv("SCRIPT_NAME") << std::endl;
	std::cout << "PATH_INFO: " << getenv("PATH_INFO") << std::endl;
	std::cout << "SERVER_NAME: " << getenv("SERVER_NAME") << std::endl;
	std::cout << "SERVER_PORT: " << getenv("SERVER_PORT") << std::endl;
	std::cout << "HTTP_HOST: " << getenv("HTTP_HOST") << std::endl;
	std::cout << "HTTP_USER_AGENT: " << getenv("HTTP_USER_AGENT") << std::endl;
	std::cout << "HTTP_ACCEPT: " << getenv("HTTP_ACCEPT") << std::endl;
	std::cout << "HTTP_ACCEPT_LANGUAGE: " << getenv("HTTP_ACCEPT_LANGUAGE") << std::endl;
	std::cout << "HTTP_ACCEPT_ENCODING: " << getenv("HTTP_ACCEPT_ENCODING") << std::endl;
	std::cout << "HTTP_ACCEPT_CHARSET: " << getenv("HTTP_ACCEPT_CHARSET") << std::endl;
	std::cout << "HTTP_KEEP_ALIVE: " << getenv("HTTP_KEEP_ALIVE") << std::endl;
	std::cout << "HTTP_CONTENT_TYPE: " << getenv("HTTP_CONTENT_TYPE") << std::endl;
	std::cout << "HTTP_CONTENT_LENGTH: " << getenv("HTTP_CONTENT_LENGTH") << std::endl;
}

std::string	ResponseHandle::getFilePath() const {
	return _filePath;
}

std::string	ResponseHandle::getPathInfo() const {
	return _pathInfo;
}

std::string	ResponseHandle::getScriptName() const {
	return _scriptName;
}

std::string	ResponseHandle::getHttpUri() const {
	return _httpUri;
}

std::string	ResponseHandle::getServerRoot() const {
	return _serverRoot;
}

Port	ResponseHandle::getPort() const {
	return _port;
}

const LocationConfig&	ResponseHandle::getLocation() const {
	return _loc;
}

ResponseHandle&	ResponseHandle::operator=(const ResponseHandle &Copy) {
	if (this == &Copy)
		return *this;
	_response = Copy._response;
	_isInitFromLocation = Copy._isInitFromLocation;
	_filePath = Copy._filePath;
	_pathInfo = Copy._pathInfo;
	_scriptName = Copy._scriptName;
	_httpUri = Copy._httpUri;
	_serverRoot = Copy._serverRoot;
	_port = Copy._port;
	_loc = Copy._loc;
	return *this;
}

