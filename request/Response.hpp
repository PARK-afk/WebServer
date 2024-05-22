#pragma once

#include <string>
#include <map>
#include <vector>
#include <sstream>
#include "../utils/utils.hpp"
#include "../utils/Error.hpp"
#include <iostream>

class Response {
private:
	std::string _version;
	int _status;
	std::string _reason;
	std::string _body;
	std::string _location;
public:
    typedef std::multimap<std::string, std::string> Headers;
    Headers headers;

	Response();
	~Response();

	void setVersion(const std::string& version);

	std::string getVersion() const;

	int getStatusCode() const;

	std::string getReason() const;

    void setHeader(const std::string& name, const std::string& value);

	void setRedirect(const std::string& url, int status = 302);

	void setStatusCode(int status);
		

    void removeHeader(const std::string& name);

    std::string getHeaderValue(const std::string& name) const;
    std::vector<std::string> getHeaderValues(const std::string& name) const;

    bool hasHeader(const std::string& name) const;

	void setBody(const std::string& body);

	const std::string getResponses();

	Response &operator=(const Response &Copy);
	void	clearAll();
};
