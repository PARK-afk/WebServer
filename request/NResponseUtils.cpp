#include "NResponseUtils.hpp"

ResponseUtils::ResponseUtils() {}

ResponseUtils::~ResponseUtils() {}

bool    ResponseUtils::isMethodPossible(int method, const LocationConfig &Loc) {
    for (size_t i = 0; i < Loc.getAllowMethods().size(); ++i) {
        if (method == ResponseUtils::getMethodNumber(Loc.getAllowedMethod(i))) {
            return true;
        }
    }
    return false;
}

bool ResponseUtils::isValidPath(const std::string &path)
{
	if (path.empty())
	{
		return false;
	}
	if (path.length() > PATH_MAX)
	{
		return false;
	}
	if (path.find_first_of("\0\\") != std::string::npos)
	{
		return false;
	}
	if (path[0] != '/')
	{
		return false;
	}
	return true;
}

bool ResponseUtils::isExtention(std::string httpPath)
{
	if (httpPath.find_last_of('.') == std::string::npos)
		return false;
	return true;
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
	
	std::string::size_type pos = 0;
	while ((pos = normalizedUrl.find("//", pos)) != std::string::npos) {
		normalizedUrl.erase(pos, 1);
	}
	return normalizedUrl;
}

std::string ResponseUtils::normalizePath(const std::string &path) {
    std::string normalizedPath = path;
    
    std::string::size_type pos = 0;
    while ((pos = normalizedPath.find("//", pos)) != std::string::npos) {
        normalizedPath.erase(pos, 1);
    }
    
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
    
    while ((pos = normalizedPath.find("/./")) != std::string::npos) {
        normalizedPath.erase(pos, 2);
    }
    
    return normalizedPath;
}

bool ResponseUtils::isExist(const std::string &path) {
	struct stat buf;
	if (stat(path.c_str(), &buf) == 0) {
		return true;
	}
	return false;
}


std::string ResponseUtils::generateETag(const std::string& content) {
        unsigned long hash = 5381;
        int c;
        for (size_t i = 0; i < content.length(); ++i) {
            c = content[i];
            hash = ((hash << 5) + hash) + c;
        }
        std::ostringstream oss;
        oss << "\"" << hash << "\"";
        return oss.str();
}

std::string ResponseUtils::getLastModified(const std::string& filePath)
{
	struct stat fileInfo;
	if (stat(filePath.c_str(), &fileInfo) == 0) {
		char buffer[80];
		strftime(buffer, sizeof(buffer), "%a, %d %b %Y %H:%M:%S GMT", gmtime(&fileInfo.st_mtime));
		return std::string(buffer);
    }
    return "";
}

bool ResponseUtils::isDirectory(const std::string &path)
{
	struct stat st;

	if (stat(path.c_str(), &st) == 0)
	{
		if (S_ISREG(st.st_mode))
		{
			return false;
		}
		return S_ISDIR(st.st_mode);
	}

	return false;
}

std::string ResponseUtils::lastModify(const std::string& filePath) {
    struct stat fileStat;
    if (stat(filePath.c_str(), &fileStat) == 0) {
        std::time_t lastModifiedTime = fileStat.st_mtime;
        std::stringstream ss;
        ss << std::put_time(std::gmtime(&lastModifiedTime), "%a, %d %b %Y %H:%M:%S GMT");
        return ss.str();
    }
    return "";
}

std::string ResponseUtils::getExpirationTime(int seconds) {
	std::time_t now = std::time(0) + seconds;
	std::stringstream ss;
	ss << std::put_time(std::gmtime(&now), "%a, %d %b %Y %H:%M:%S GMT");
	return ss.str();
}

std::string ResponseUtils::etag(const std::string& filePath) {
	struct stat fileStat;
	if (stat(filePath.c_str(), &fileStat) == 0) {
		std::string etag = std::to_string(fileStat.st_ino) + std::to_string(fileStat.st_size) + std::to_string(fileStat.st_mtime);
		return generateETag(etag);
	}
	return "";
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
	const char *sizes[] = {"B", "KB", "MB", "GB", "TB"};
	int i = 0;
	while (size > 1024)
	{
		size /= 1024;
		i++;
	}

	std::ostringstream oss;
	oss << std::fixed << std::setprecision(2) << size << sizes[i];
	return oss.str();
}

std::string ResponseUtils::getFileExtension(const std::string &filePath)
{
	size_t dotPos = filePath.find_last_of('.');
	if (dotPos != std::string::npos)
	{
		return filePath.substr(dotPos + 1);
	}
	return "";
}

std::streamsize ResponseUtils::getFileSize(std::ifstream &file)
{
	file.seekg(0, std::ios::end);
	std::streamsize fileSize = file.tellg();
	file.seekg(0, std::ios::beg);
	return fileSize;
}

std::string ResponseUtils::readFileContent(std::ifstream &file, std::streamsize fileSize)
{
	std::string content(fileSize, '\0');
	file.read(&content[0], fileSize);
	return content;
}

std::string ResponseUtils::getContentType(const std::string &extension)
{
	if (extension == "html")
		return "text/html; charset=utf-8";
	else if (extension == "css")
		return "text/css";
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



