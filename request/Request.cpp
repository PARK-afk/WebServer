#include "Request.hpp"

void HttpRequest::isVaildRequest(const Request &req)
{
	if (req._method == "OTHER")
		throw std::invalid_argument("Invalid Method");
	if (req._uri.empty())
		throw std::invalid_argument("Invalid URI");
	if (req._version != "HTTP/1.1")
		throw std::invalid_argument("Invalid Version");
	if (req._headers.find("Host") == req._headers.end())
		throw std::invalid_argument("Invalid Host");
}

std::string parseMethod(const std::string& methodStr)
{
	if (methodStr == "GET")
		return "GET";
	else if (methodStr == "POST")
		return "POST";
	else if (methodStr == "DELETE")
		return "DELETE";
	else
		return "OTHER";
}

bool isQuary(const std::string &uri)
{
	if (uri.find('?') != std::string::npos)
		return true;
	return false;
}

std::string parseUri(const std::string& uri, Request &req)
{
	std::string path;

	if (isQuary(uri)) {
			path = uri.substr(0, uri.find('?'));
			req._query = uri.substr(uri.find('?') + 1);
	}
	else 
		path = uri;
	return path;
}

void HttpRequest::parseRequestLine(Request &req, const std::string& line)
{
    std::istringstream iss(line);
	std::string token, tmpUri;

	iss >> token;
	if (iss.fail())
		throw std::invalid_argument("Invalid Request Method");
	req._method = parseMethod(token);

	iss >> tmpUri;
	if (iss.fail())
		throw std::invalid_argument("Invalid Request URI");
	req._uri = parseUri(tmpUri, req);

	iss >> req._version;
	if (iss.fail())
		throw std::invalid_argument("Invalid Request Version");
}

void HttpRequest::parseHeader(Request &req, const std::string& header)
{
	std::istringstream iss(header);
	std::string line;

	while (std::getline(iss, line))
	{
		std::string::size_type pos = line.find(":");
		if (pos != std::string::npos) {
			std::string key = line.substr(0, pos);
			std::string value = line.substr(pos + 2);
			req._headers[key] = value;
		}
	}
}

void HttpRequest::setCookie(Request &req)
{
	std::istringstream iss(req._headers["Cookie"]);
	std::string line;

	while (std::getline(iss, line, ';')) {
		std::string::size_type pos = line.find("=");
		if (pos != std::string::npos) {
			std::string key = line.substr(0, pos);
			std::string value = line.substr(pos + 1);
			req._cookie[key] = value;
		}
	}
}

std::string HttpRequest::parsePart(const std::string& body, const std::string& boundary) {
    size_t start = body.find(boundary);
    if (start == std::string::npos) {
        return "";
    }
    start += boundary.length();
    size_t end = body.find(boundary, start);
    if (end == std::string::npos) {
        return "";
    }
    return body.substr(start, end - start);
}

std::string HttpRequest::parseFileContent(const std::string &body) {
    size_t start = body.find("\r\n\r\n");
    if (start == std::string::npos) {
        return "";
    }
    start += 4;
    return body.substr(start);
}

std::string HttpRequest::parseBodyHeader(const std::string& part) {
    size_t end = part.find("\r\n\r\n");
    if (end == std::string::npos) {
        return "";
    }
    return part.substr(0, end);
}

std::string HttpRequest::parseType(const std::string& body_header) {
    size_t start = body_header.find("Content-Type: ");
    if (start == std::string::npos) {
        return "";
    }
    start += 14;
    size_t end = body_header.find("\r\n", start);
    return body_header.substr(start, end - start);
}

std::string HttpRequest::parseFileName(const std::string& body_header) {
    size_t start = body_header.find("filename=\"");
    if (start != std::string::npos) {
        start += 10;
        size_t end = body_header.find("\"", start);
        return body_header.substr(start, end - start);
    }
    return "";
}

std::string HttpRequest::parseBoundary(const std::string& body_header) {
    size_t start = body_header.find("boundary=");
    if (start == std::string::npos) {
        return "";
    }
    start += 9;
    return body_header.substr(start);
}

std::string HttpRequest::parseContentType(std::string &body_header)
{
    size_t start = body_header.find("Content-Type: ");
    if (start == std::string::npos)
        return "";
    start += 14;
    size_t end = body_header.find(";", start);
    return body_header.substr(start, end - start);
}
