#include "Client.hpp"
#include "../tmp/test.hpp"
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <unistd.h>

Client::Client(Port port) : _port(port) {
    clearAll();
}

Client::Client(const Client &Copy) : 
    _buffer(Copy.getBuffer()),
    _request(Copy.getRequest()),
    _port(Copy.getPort()),
    _readStatus(Copy._readStatus),
    _responseStatus(Copy._responseStatus) {}

Client::~Client() {}

std::string Client::getBuffer() const {
    return _buffer;
}

std::string Client::getMethod() const {
    return _request._method;
}

std::string Client::getUri() const {
    return _request._uri;
}

std::string Client::getVersion() const {
    return _request._version;
}

Port Client::getPort() const {
    return _port;
}

std::string Client::getHeader(const std::string& key) const {
    const Headers::const_iterator it = _request._headers.find(key);
    if (it == _request._headers.end())
        return "";
    return it->second;
}

std::string Client::getBody() const {
    return _request._body;
}

std::string Client::getCookie(const std::string& key) const {
    const Cookies::const_iterator it = _request._cookie.find(key);
    if (it == _request._cookie.end())
        return "";
    return it->second;
}

const Request &Client::getRequest() const {
    return _request;
}

int Client::getResponseStatus() const {
    return _responseStatus;
}

void Client::setBuffer(const std::string& buffer) {
    _buffer += buffer;
    setRequest();
}

void Client::setRequest() {
    std::istringstream iss(_buffer);
	std::string line, header, body;
    int contentLength = 0;

	try {
        if (iss.str().find("\r\n") != std::string::npos \
                && _readStatus == READ_NOT_DONE) {
    		std::getline(iss, line);
	    	HttpRequest::parseRequestLine(_request, line);
            _readStatus = READ_LINE_DONE;
        }

		size_t pos = iss.str().find("\r\n\r\n");
        if (pos == std::string::npos &&\
            _readStatus == READ_LINE_DONE)
            return ;

		if (pos != std::string::npos &&\
             _readStatus == READ_LINE_DONE)
		{
			header = iss.str().substr(0, pos);
            HttpRequest::parseHeader(_request, header);
            if (_request._headers.find("Content-Length") != _request._headers.end())
                _request._contentLength = atoi(_request._headers["Content-Length"].c_str());
            else
                _request._contentLength = 0;
            if (_request._headers.find("Cookie") != _request._headers.end())
                HttpRequest::setCookie(_request);
            contentLength = _request._contentLength;
            _readStatus = READ_HEADER_DONE;
		}

		if ((_readStatus == READ_HEADER_DONE || \
                _readStatus == READ_BODY_DOING) && contentLength > 0)
        {
            body = iss.str().substr(pos + 4);
            contentLength -= body.length();
            if (contentLength > 0) {
                _readStatus = READ_BODY_DOING;
                return ;
            }
            else if (contentLength == 0)
                _readStatus = READ_DONE;
            else if (contentLength < 0)
                line.substr(0, line.length() + contentLength);
		}
        else if (_readStatus == READ_HEADER_DONE && \
                    _request._contentLength == 0) {
            _readStatus = READ_DONE;
        }
        _request._body += HttpRequest::parseBody(body);
		HttpRequest::isVaildRequest(_request);
        _responseStatus = 200;
	}
	catch (std::invalid_argument& e) {
        std::cerr << "Exception caught: " << e.what() << std::endl;
		_readStatus = READ_ERROR;
        _responseStatus = 400;
	}
}

void Client::clearRequest()
{
    _request._method.clear();
    _request._uri.clear();
    _request._version.clear();
    _request._headers.clear();
    _request._cookie.clear();
    _request._body.clear();
    _request._contentLength = 0;
}

void Client::clearAll()
{
    clearRequest();
    _buffer.clear();
    _readStatus = READ_NOT_DONE;
    _responseStatus = 0;

    //for test
    _tempResult = "";
}

void Client::printAllHeaders() const{
    const Headers& headers = _request._headers;
    for (Headers::const_iterator it = headers.begin(); it != headers.end(); ++it) {
        std::cout << it->first << ": " << it->second << std::endl;
    }
}



#include <sys/stat.h>
#include <unistd.h>
#include <climits>
#include <cstring>

bool isDirectory(const std::string &path) {
    struct stat st;
    if (stat(path.c_str(), &st) == 0) {
        return S_ISDIR(st.st_mode);
    }
    return false;
}

bool isValidPath(const std::string &path) {
    // 경로가 비어있는 경우
    if (path.empty()) {
        return false;
    }
    
    // 경로가 너무 긴 경우
    if (path.length() > PATH_MAX) {
        return false;
    }
    
    // 경로에 불법적인 문자가 포함된 경우
    if (path.find_first_of("\0\\") != std::string::npos) {
        return false;
    }
    
    // 경로가 상대경로인 경우
    if (path[0] != '/') {
        return false;
    }
    
    return true;
}

std::string normalizePath(const std::string &path) {
    std::string normalizedPath = path;
    
    // 연속된 슬래시를 하나의 슬래시로 치환
    std::string::size_type pos = 0;
    while ((pos = normalizedPath.find("//", pos)) != std::string::npos) {
        normalizedPath.erase(pos, 1);
    }
    
    // 경로의 마지막 문자가 '/'인 경우 제거
    if (!normalizedPath.empty() && normalizedPath[normalizedPath.length() - 1] == '/') {
        normalizedPath.erase(normalizedPath.length() - 1);
    }
    
    // 상대경로 요소 제거
    while ((pos = normalizedPath.find("/../")) != std::string::npos) {
        if (pos == 0) {
            return "";
        }
        std::string::size_type prevPos = normalizedPath.rfind('/', pos - 1);
        if (prevPos == std::string::npos) {
            return "";
        }
        normalizedPath.erase(prevPos, pos - prevPos + 3);
    }
    
    // 현재 디렉토리 요소 제거
    while ((pos = normalizedPath.find("/./")) != std::string::npos) {
        normalizedPath.erase(pos, 2);
    }
    
    return normalizedPath;
}

int getMethodNumber(const std::string &method) {
	if (method == "GET")
		return 1;
	else if (method == "POST")
		return 2;
	else if (method == "PUT")
		return 3;
	else if (method == "DELETE")
		return 4;
	return 0;
}

#include <sys/time.h>

std::string getCurTime() {
	struct timeval tv;
	struct tm *tm;
	char buf[64];

	gettimeofday(&tv, NULL);
	tm = localtime(&tv.tv_sec);
	strftime(buf, sizeof(buf), "%a, %d %b %Y %H:%M:%S GMT", tm);
	return std::string(buf);
}

bool isExist(const std::string& path, const std::string& file)
{
	std::string full_path = path + file;
	std::ifstream ifs(full_path.c_str());
	
	return ifs.is_open();
}

std::string getContentType(const std::string &extension) {
    if (extension == "html")
        return "text/html; charset=utf-8";
    else if (extension == "css")
        return "text/css";
    // 다른 확장자에 대한 Content-Type 매핑 추가
    else if (extension == "png")
		return "image/png";
	else if (extension == "jpg")
		return "image/jpeg";
	else if (extension == "jpeg")
		return "image/jpeg";
	else if (extension == "gif")
		return "image/gif";
	else if (extension == "bmp")
		return "image/bmp";
	else if (extension == "ico")
		return "image/x-icon";
	else if (extension == "svg")
		return "image/svg+xml";
	else if (extension == "js")
		return "application/javascript";
	else if (extension == "json")
		return "application/json";
	else if (extension == "pdf")
		return "application/pdf";
	else if (extension == "zip")
		return "application/zip";
	else
        return "application/octet-stream";
}

Response Client::handleGetRequest(const Config &Conf) {
    Response response;
    (void)Conf;
    // GET 요청 처리 로직
    const std::string url = this->getUri();
    std::string filePath = "/Users/seodong-gyun/42/webserver/WebServer/html";
    filePath += url;

    filePath = normalizePath(filePath);
    if (!isValidPath(filePath)) {
        std::cout << "Invalid request path: " << filePath << std::endl;
        response.setStatusCode(BadRequest_400);
        response.setHeader("Content-Type", "text/html; charset=utf-8");
        response.setHeader("Date", getCurTime());
        std::string errorBody = "<html><body><h1>400 Bad Request</h1><p>Invalid request path.</p></body></html>";
        response.setBody(errorBody);
        response.setHeader("Content-Length", std::to_string(errorBody.length()));
        response.setHeader("Connection", "close");
        return response;
    }

    // 디렉토리 접근 제한
    if (filePath.find("/../") != std::string::size_type(-1) || !isExist(filePath, "")) {
        response.setStatusCode(Forbidden_403);
        response.setHeader("Content-Type", "text/html; charset=utf-8");
        response.setHeader("Date", getCurTime());
        std::string errorBody = "<html><body><h1>403 Forbidden</h1><p>Access to the requested resource is forbidden.</p></body></html>";
        response.setBody(errorBody);
        response.setHeader("Content-Length", std::to_string(errorBody.length()));
        response.setHeader("Connection", "close");
        return response;
    }

    if (url == "/") {
        filePath += "/index.html";
    }

    std::string extension = filePath.substr(filePath.find_last_of(".") + 1);
    std::ifstream file(filePath.c_str(), std::ios::binary);

    if (file.is_open()) {
        // 파일 크기 확인
        file.seekg(0, std::ios::end);
        std::streamsize fileSize = file.tellg();
        file.seekg(0, std::ios::beg);

        // 파일 크기 제한 (예: 10MB)
        const std::streamsize maxFileSize = 10 * 1024 * 1024;
        if (fileSize > maxFileSize) {
            response.setStatusCode(PayloadTooLarge_413);
            response.setHeader("Content-Type", "text/html; charset=utf-8");
            response.setHeader("Date", getCurTime());
            std::string errorBody = "<html><body><h1>413 Request Entity Too Large</h1><p>The requested file is too large.</p></body></html>";
            response.setBody(errorBody);
            response.setHeader("Content-Length", std::to_string(errorBody.length()));
            response.setHeader("Connection", "close");
            return response;
        }

        std::string body(fileSize, '\0');
        file.read(&body[0], fileSize);

        response.setStatusCode(OK_200);
        response.setHeader("Date", getCurTime());
        response.setHeader("Content-Type", getContentType(extension));
        response.setBody(body);
        response.setHeader("Content-Length", std::to_string(body.length()));
        response.setHeader("Connection", "keep-alive");

        file.close();
    } else {
        response.setStatusCode(NotFound_404);
        response.setHeader("Content-Type", "text/html; charset=utf-8");
        response.setHeader("Date", getCurTime());
        std::string errorBody = "<html><body><h1>404 Not Found</h1><p>The requested file was not found.</p></body></html>";
        response.setBody(errorBody);
        response.setHeader("Content-Length", std::to_string(errorBody.length()));
        response.setHeader("Connection", "close");
    }

    response.setHeader("Server", "42Webserv");
    response.setHeader("X-Content-Type-Options", "nosniff");
    response.setHeader("X-Frame-Options", "SAMEORIGIN");
    response.setHeader("X-XSS-Protection", "1; mode=block");

    return response;
}

Response handleMethodNotAllowed() {
    Response response;
    response.setStatusCode(MethodNotAllowed_405);
    response.setHeader("Content-Type", "text/html");
    response.setHeader("Date", getCurTime());

    std::string errorBody = "<html><body><h1>405 Method Not Allowed</h1><p>The requested method is not allowed.</p></body></html>";
    response.setBody(errorBody);
    response.setHeader("Content-Length", std::to_string(errorBody.length()));
    response.setHeader("Connection", "close");

    return response;
}

Response Client::sendResponse(const Config &Conf) {
    enum Method {
        GET = 1,
        POST,
        PUT,
        DELETE
    };

    int method = getMethodNumber(_request._method);
    switch (method) {
        case GET:
            return handleGetRequest(Conf);
        case POST:
            // return handlePostRequest(Conf);
        case PUT:
            // return handlePutRequest(Conf);
        case DELETE:
            // return handleDeleteRequest(Conf);
        default:
            return handleMethodNotAllowed();
    }
}





std::string Client::execute(const Config &Conf) {
	Response response = sendResponse(Conf);
	std::string responseStr = response.get_responses();
	_tempResult = responseStr;
	return responseStr;
}