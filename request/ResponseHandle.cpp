#include "ResponseHandle.hpp"


ResponseHandle::ResponseHandle() {

}

ResponseHandle::ResponseHandle(const ResponseHandle &Copy) : _response(Copy._response) {}

ResponseHandle::~ResponseHandle() {}

void ResponseHandle::generateResponse(const RequestHandle &Req, Config &Conf) {
	if (initPathFromLocation(Req, Conf) == false) {
		return ;
	}
	std::cout << "Start to generate response" << std::endl;
	int method = ResponseUtils::getMethodNumber(Req.getMethod());
    std::cout << "method number = " << method << "string = " << Req.getMethod() << "123" << std::endl;
	switch (method)
	{
		case GET:
			_response = handleGetRequest(Req);
            std::cout << "Goto GET" << std::endl;
			break;
		case POST:
			// _response = handlePostRequest(Conf);
            std::cout << "Goto POST" << std::endl;
			break;
		case DELETE:
			_response = handleDeleteRequest();
            std::cout << "Goto DELETE" << std::endl;
			break;
		default:
			_response = createErrorResponse(MethodNotAllowed_405, "The requested method is not allowed.");
            std::cout << "Goto ERROR" << std::endl;
			break;
	}
}

void ResponseHandle::clearAll() {
	_response.clearAll();
}
void printAllEnv();
const std::string ResponseHandle::getResponse() {
	return _response.getResponses();
}

Response ResponseHandle::createErrorResponse(int code, const std::string &message) {
	Response response;
	response.setStatusCode(code);
	response.setHeader("Content-Type", "text/html; charset=utf-8");
	response.setHeader("Date", ResponseUtils::getCurTime());
	std::string errorBody = "<html><body><h1>" + web::toString(code) + " " + Error::errors[code] + "</h1><p>" + message + "</p></body></html>";
	response.setBody(errorBody);
	response.setHeader("Content-Length", web::toString(errorBody.length()));
	response.setHeader("Connection", "close");
	return response;

}




Response ResponseHandle::handleMethodNotAllowed() {
    Response response;
    response.setStatusCode(MethodNotAllowed_405);
    response.setHeader("Content-Type", "text/html");
    response.setHeader("Date", ResponseUtils::getCurTime());

    std::string errorBody = "<html><body><h1>405 Method Not Allowed</h1><p>The requested method is not allowed.</p></body></html>";
    response.setBody(errorBody);
    response.setHeader("Content-Length", web::toString(errorBody.length()));
    response.setHeader("Connection", "close");

    return response;
}

bool	ResponseUtils::isExtention(std::string httpPath) { 
    if (httpPath.find_last_of('.') == std::string::npos)
        return false;
    return true;
}

std::string ResponseHandle::getFilePath(const std::string &serverRoot, const std::string &httpUri, LocationConfig &loc) {
    std::string filePath;
    std::string alias = loc.getAlias();
    std::cout << "in getFilePath : ServerRoot = " << serverRoot << std::endl;
    std::cout << "in getFilePath : httpUri = " << httpUri << std::endl;
    std::cout << "loc.getRoot : " << loc.getRoot() << std::endl;
    if (!alias.empty() && httpUri.find(alias) == 0) {
        filePath = alias + httpUri.substr(alias.length());
        std::cout << "1" << std::endl;
    }
    else if (ResponseUtils::isExtention(httpUri) == true) {
        std::cout << "2" << std::endl;
		if (loc.getFastcgiPass().empty() && loc.getPath().find('.') != std::string::npos) {
            std::cout << "in getFilePath : " << __LINE__ << std::endl;
        	filePath = serverRoot + loc.getRoot() + httpUri;
		} else if (loc.getFastcgiPass().empty() && loc.getPath().find('.') == std::string::npos) {
            filePath = serverRoot + httpUri;
        } else {
			std::cout << "is CGI" << std::endl;
            std::cout << "in getFilePath : " << __LINE__ << std::endl;
			loc.setCgi(true);
			_scriptName = httpUri.substr(0, httpUri.find_last_of('/'));
			_pathInfo = httpUri.substr(httpUri.find_last_of('/'));
			_httpUri = _scriptName;
			setenv("SCRIPT_NAME", _scriptName.c_str(), 1);
			setenv("PATH_INFO", _pathInfo.c_str(), 1);
			filePath = serverRoot + loc.getFastcgiPass() + httpUri;
		}
    } else if (ResponseUtils::isDirectory(serverRoot + httpUri) == true) {
            std::cout << "in getFilePath : " << __LINE__ << std::endl;
        filePath = serverRoot + httpUri;
    } else {
        std::cout << "4" << std::endl;
        filePath = serverRoot + loc.getRoot() + httpUri.substr(loc.getRoot().length(), httpUri.length() - loc.getRoot().length());
    }

    return filePath;
}

bool ResponseUtils::isValidPath(const std::string &path) {
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

Response ResponseHandle::handleRedirect(const LocationConfig &location) {
    Response response;
    std::string returnCode = location.getReturnCode();
    std::string returnUrl = location.getReturnUrl();

    if (!returnCode.empty() && !returnUrl.empty()) {
		std::cout << "Redirected to: " << returnUrl << std::endl;
        int statusCode = std::stoi(returnCode);
		response.setRedirect(returnUrl, statusCode);
        response.setHeader("Connection", "close");
		return response;
    }
	response.setStatusCode(OK_200);
    return response;
}

bool ResponseUtils::isDirectory(const std::string &path) {
    struct stat st;

    if (stat(path.c_str(), &st) == 0) {
        if (S_ISREG(st.st_mode)) {
            return false;  // 파일인 경우 false 반환
        }
        return S_ISDIR(st.st_mode);
    }

    return false;
}

std::string ResponseUtils::getFileExtension(const std::string &filePath) {
    size_t dotPos = filePath.find_last_of('.');
    if (dotPos != std::string::npos) {
        return filePath.substr(dotPos + 1);
    }
    return "";
}

std::streamsize ResponseUtils::getFileSize(std::ifstream &file) {
    file.seekg(0, std::ios::end);
    std::streamsize fileSize = file.tellg();
    file.seekg(0, std::ios::beg);
    return fileSize;
}

std::string ResponseUtils::readFileContent(std::ifstream &file, std::streamsize fileSize) {
    std::string content(fileSize, '\0');
    file.read(&content[0], fileSize);
    return content;
}

std::string ResponseUtils::getContentType(const std::string &extension) {
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

bool	ResponseHandle::initPathFromLocation(const RequestHandle &Req, Config &Conf) {
	_httpUri = Req.getUri();
	_port = Req.getPort();
	// Conf.setServerName(Req.getHost());

	// URL 정규화
	_httpUri = ResponseUtils::nomralizeUrl(_httpUri);
	std::cout << "Normalized URL: " << _httpUri << std::endl;
	_serverRoot = ResponseUtils::normalizePath(Conf.getServerConfig(_port, Req.getHost()).getPath());
	if (_serverRoot.empty()) {
		_response = createErrorResponse(InternalServerError_500, "Server configuration error: root directory not set.");
		return false;
    }

	try {
		_loc = Conf.getServerConfig(_port, Req.getHost()).getLocation(_httpUri);
		std::cout << "Success to get location "<< _loc.getPath() << std::endl;
	} catch (const std::exception &e) {

		std::cout << "Failed to get location" << std::endl;
		_loc = Conf.getServerConfig(_port, Req.getHost()).getLocation("/");
	}
	if (ResponseUtils::isMethodPossible(GET, _loc) == false) {
		_response = createErrorResponse(MethodNotAllowed_405, "The requested method is not allowed.");
		return false;
	}
	_filePath = getFilePath(_serverRoot, _httpUri, _loc);
	if (!ResponseUtils::isValidPath(_filePath)) {
		_response = createErrorResponse(BadRequest_400, "Invalid request path.");
		return false;
	}
	return true;
}

Response ResponseHandle::handleGetRequest(const RequestHandle &Req) {
    Response response;
	if (_loc.isCgi() == true) {
		// CGI 처리
		setEnv(Req);
		printAllEnv();
		
		std::cout << "Start to handle CGI" << std::endl;
		// response = handleCgi(_loc, _filePath);
	}

	std::cout << "Start to handle GET request" << std::endl;
	// 리다이렉트 처리
	
	Response redirectResponse = handleRedirect(_loc);
	if (redirectResponse.getStatusCode() != OK_200) {
		return redirectResponse;
	}

    // 인덱스 파일 설정
	std::cout << "Start to get file" << std::endl;
    std::string index = _loc.getIndex();
    if (ResponseUtils::isDirectory(_filePath) && !index.empty()) {
        _filePath += "/" + index;
    }

    // 파일 확장자 추출
	std::cout << "Start to get file extension" << std::endl;
    std::string extension = ResponseUtils::getFileExtension(_filePath);
    // 파일 읽기
    std::ifstream file(_filePath.c_str(), std::ios::binary);
	std::cout << "File Path: " << _filePath << std::endl;
    if (file.is_open() && file.good()) {
        // 파일 크기 확인
        std::streamsize fileSize = ResponseUtils::getFileSize(file);

        // 파일 크기 제한 설정
        const std::streamsize maxFileSize = 10 * 1024 * 1024;
        if (fileSize > maxFileSize) {
			return createErrorResponse(PayloadTooLarge_413, "The requested file is too large.");
        }
        // 파일 내용 읽기
        std::string body = ResponseUtils::readFileContent(file, fileSize);
        file.close();

        response.setStatusCode(OK_200);
        response.setHeader("Date", ResponseUtils::getCurTime());
        response.setHeader("Content-Type", ResponseUtils::getContentType(extension));
        response.setBody(body);
        response.setHeader("Content-Length", web::toString(body.length()));
        response.setHeader("Connection", "keep-alive");

    } else {
        if (ResponseUtils::isDirectory(_filePath)) {
			if (_loc.getAutoindex() == true) {
            	handleAutoIndex(response, _filePath);
			} else {
				return createErrorResponse(Forbidden_403, "Directory listing not allowed.");
			}
        } else {
			if (_loc.getAutoindex() == false) {
				return createErrorResponse(NotFound_404, "The requested file was not found.");
			} else {
				handleAutoIndex(response, _filePath.substr(0, _filePath.find_last_of('/')));
			}
		}
    }
    response.setHeader("Server", "42Webserv");
    return response;
}

std::string ResponseHandle::handlePostRequest(const RequestHandle &Req) {
    
    std::string contentType = Req.parseHeader("Content-Type");
    if (contentType.find("multipart/form-data") != std::string::npos) {
        
        const std::string part = parsePart(_body, parseBoundary(contentType));
        const std::string bodyHeader = parseBodyHeader(part);
        if (bodyHeader.empty())
            return createErrorResponse(BadRequest_400, "Bad Request");
        std::string fileName = parseFileName(bodyHeader);
        if (fileName.empty())
            return createErrorResponse(BadRequest_400, "Bad Request");

        const std::string fileContent = parsefileContent(part);
        if (fileContent.empty())
            return createErrorResponse(BadRequest_400, "Bad Request");

        const std::streamsize maxFileSize = 10 * 1024 * 1024;
        if (fileContent.size() > maxFileSize)
            return createErrorResponse(UriTooLong_414, "Body too large");

        if (!fileContent.empty()) {
            std::string responseData = handleFormData(_filePath);

            if (responseData.empty())
                return createErrorResponse(InternalServerError_500, "Internal Server Error");

            std::ifstream file(fileName);
            if (!file.good())
                return createErrorResponse(InternalServerError_500, "Internal Server Error");
            file.close();
        }
    }
    else if (contentType.find("application/x-www-form-urlencoded") != std::string::npos)
    {
        std::string responseData = handleFormData(_filePath);

        if (responseData.empty()) {
            return createErrorResponse(InternalServerError_500, "Internal Server Error");
        }
    }
    else {
        return createErrorResponse(BadRequest_400, "Bad Request");
    }
    return responseData;
}

std::string ResponseHandle::handleFormData(const std::string &cgiPath, const RequestHandle &Req) {
    int cgiInput[2];
    pid_t pid;

    if (pipe(cgiInput) < 0)
        return "";
    
    if ((pid = fork()) < 0)
        return "";

    if (pid == 0) {
        setEnv(Req);
        close(cgiInput[0]);
        dup2(cgiInput[1], STDOUT_FILENO);
        close(cgiInput[1]);

        // std::string pythonPath = "/Library/Frameworks/Python.framework/Versions/3.11/lib/python3.11/site-packages";
        // std::string pythonEnv = "PYTHONPATH=" + pythonPath;
        // char* envp[] = {(char*)pythonEnv.c_str(), NULL};
        // 혹시 몰라 일단 남겨둠 -> python 실행 해보고 필요해지면 추가 할 예정

        std::string py3 = "/usr/bin/python3"; //
        char* const arg[] = {(char *)py3.c_str(), (char *)cgiPath.c_str(), NULL};

        if (execve(py3.c_str(), arg, NULL) == -1) {
            perror("execve failed");
            exit(404);
        }
    } else {
        int status;
        close(cgiInput[1]);
        waitpid(pid, &status, 0);
        // if (status == 404)
        //     return "";

        std::string output;
        char buf[1024];
        ssize_t bytesRead;
        while ((bytesRead = read(cgiInput[0], buf, sizeof(buf))) > 0) {
            output.append(buf, bytesRead);
        } // --> nonblocking
        close(cgiInput[0]);
        return output;
    }
}

std::string ResponseUtils::getFormattedTime(time_t time)
{
	char buffer[80];
	struct tm *timeinfo = localtime(&time);
	strftime(buffer, 80, "%Y-%m-%d %H:%M:%S", timeinfo);
	return std::string(buffer);
}

std::string ResponseUtils::getFormatSize(double size)
{
	const char *sizes[] = { "B", "KB", "MB", "GB", "TB" };
	int i = 0;
	while (size > 1024)
	{
		size /= 1024;
		i++;
	}

	std::ostringstream oss;
	oss << std::fixed << std::setprecision(2) <<size << sizes[i];
	return oss.str();
}

void ResponseHandle::handleAutoIndex(Response &response, const std::string &servRoot)
{
    std::string dirPath = servRoot;

    struct stat fileStat;
    std::stringstream body;
    body << "<html>\n<head>\n<title> AutoIndex </title>\n</head>\n<body>\n";
    body << "<h1>Index of / </h1>\n";
	body << "<hr> <pre>\n<table>\n<tr><th></th><th></th><th></th></tr>\n";

    DIR *dir = opendir(dirPath.c_str());
	if (dir == NULL) {
        response.setStatusCode(NotFound_404);
        response.setHeader("Content-Type", "text/html; charset=utf-8");
        response.setHeader("Date", ResponseUtils::getCurTime());
        std::string errorBody = "<html><body><h1>404 Not Found</h1><p>Directory listing not allowed.</p></body></html>";
        response.setBody(errorBody);
        response.setHeader("Content-Length", web::toString(errorBody.length()));
        response.setHeader("Connection", "close");
	}
    if (dir)
    {
        std::vector<std::string> fileList;
        struct dirent *ent;
        size_t maxFileNameLength = 0;
        while ((ent = readdir(dir)) != NULL) {
            fileList.push_back(ent->d_name);
            maxFileNameLength = std::max(maxFileNameLength, strlen(ent->d_name));
        }
        closedir(dir);

        std::sort(fileList.begin(), fileList.end());

        int count = fileList.size();
        for (int i = 0; i < count; i++)
        {
            std::string fileName = fileList[i];
            if (fileName == ".")
                continue;
            std::string filePath = dirPath + "/" + fileName;
            if (stat(filePath.c_str(), &fileStat) == -1) {
                response.setStatusCode(503);
                return ;
            }
            if (stat(filePath.c_str(), &fileStat) == 0)
            {
                body << "<tr>" << "<td>";
                if (S_ISDIR(fileStat.st_mode))
                    body << "<a href=\"" << fileName << "/\">" << std::left << fileName + "/" << "</a>";
                else
                    body << std::setw(maxFileNameLength + 1) << std::left << fileName;
                body << "</td><td>\t\t" << ResponseUtils::getFormattedTime(fileStat.st_mtime) << "</td>";
                double fileSize = static_cast<double>(fileStat.st_size);
                body << "<td>\t\t" << ResponseUtils::getFormatSize(fileSize) << "</td>" << "</tr>\n";
            }
            else {
                response.setStatusCode(InternalServerError_500);
                response.setHeader("Content-Type", "text/html; charset=utf-8");
                response.setHeader("Date", ResponseUtils::getCurTime());
                std::string errorBody = "<html><body><h1>500 Internal Server Error</h1><p>Directory listing not allowed.</p></body></html>";
                response.setBody(errorBody);
                response.setHeader("Content-Length", web::toString(errorBody.length())); // C++11 버전입니다.
                response.setHeader("Connection", "close");
                return ;
            }
        }
		body << " </table> </pre>\n<hr>\n</body>\n</html>\n";
        response.setStatusCode(OK_200);
        response.setHeader("Date", ResponseUtils::getCurTime());
        response.setHeader("Content-Type", "text/html");
        response.setBody(body.str());
        response.setHeader("Content-Length", web::toString(body.str().length())); // C++11 버전입니다.
        response.setHeader("Connection", "close");
    }
}



bool    ResponseUtils::isMethodPossible(int method, const LocationConfig &Loc) {
    for (size_t i = 0; i < Loc.getAllowMethods().size(); ++i) {
        if (method == ResponseUtils::getMethodNumber(Loc.getAllowedMethod(i))) {
            return true;
        }
    }
    return false;
}

std::string ResponseUtils::getCurTime() {
	struct timeval tv;
	struct tm *tm;
	char buf[64];

	gettimeofday(&tv, NULL);
	tm = localtime(&tv.tv_sec);
	strftime(buf, sizeof(buf), "%a, %d %b %Y %H:%M:%S GMT", tm);
	return std::string(buf);
}


int ResponseUtils::getMethodNumber(const std::string &method) {
	if (method == "GET")
		return GET;
	else if (method == "POST")
		return POST;
	else if (method == "DELETE")
		return DELETE;
	else if (method == "PUT")
		return PUT;
	else
		return 0;
}

std::string ResponseUtils::nomralizeUrl(const std::string &HTTP_uri) {
	std::string normalizedUrl = HTTP_uri;
	
	// //제거
	std::string::size_type pos = 0;
	while ((pos = normalizedUrl.find("//", pos)) != std::string::npos) {
		normalizedUrl.erase(pos, 1);
	}
	return normalizedUrl;	
}

std::string ResponseUtils::normalizePath(const std::string &path) {
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

void printAllEnv() {
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

void	ResponseHandle::setEnv(const RequestHandle &Req) {
	std::string host = Req.getHost();
	std::string uri = _httpUri;
	std::string scriptName = _scriptName;
	std::string query = Req.getQuery();
	std::string queryString = "";
	std::string requestUri = Req.getUri();
	std::string requestMethod = Req.getMethod();
	std::string serverName = host;
	std::string serverPort = web::toString(_port);
	setenv("REQUEST_METHOD", requestMethod.c_str(), 1);
	setenv("REQUEST_URI", requestUri.c_str(), 1);
	setenv("QUERY_STRING", query.c_str(), 1);
	setenv("SCRIPT_NAME", scriptName.c_str(), 1);
	setenv("PATH_INFO", _pathInfo.c_str(), 1);
	setenv("QUERY_STRING", query.c_str(), 1);
	setenv("SERVER_NAME", serverName.c_str(), 1);
	setenv("SERVER_PORT", serverPort.c_str(), 1);
	setenv("HTTP_HOST", host.c_str(), 1);
	setenv("HTTP_USER_AGENT", Req.getHeader("User-Agent").c_str(), 1);
	setenv("HTTP_ACCEPT", Req.getHeader("Accept").c_str(), 1);
	setenv("HTTP_ACCEPT_LANGUAGE", Req.getHeader("Accept-Language").c_str(), 1);
	setenv("HTTP_ACCEPT_ENCODING", Req.getHeader("Accept-Encoding").c_str(), 1);
	setenv("HTTP_ACCEPT_CHARSET", Req.getHeader("Accept-Charset").c_str(), 1);
	setenv("HTTP_KEEP_ALIVE", Req.getHeader("Keep-Alive").c_str(), 1);
	setenv("HTTP_CONNECTION", Req.getHeader("Connection").c_str(), 1);
	setenv("HTTP_REFERER", Req.getHeader("Referer").c_str(), 1);
	setenv("HTTP_COOKIE", Req.getHeader("Cookie").c_str(), 1);
	setenv("HTTP_CONTENT_TYPE", Req.getHeader("Content-Type").c_str(), 1);
	setenv("HTTP_CONTENT_LENGTH", Req.getHeader("Content-Length").c_str(), 1);
	setenv("BODY", Req.getBody().c_str(), 1);
}