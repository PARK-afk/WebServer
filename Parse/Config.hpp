#ifndef CONFIG_HPP
# define CONFIG_HPP

#include <vector>
#include <string>
#include <map>
#include <fstream>
#include <sstream>
#include <iostream>
#include <algorithm>
#include "ServerConfig.hpp"


class Config
{
private:
    std::vector<class ServerConfig> _servers;

public:
    Config(int argc, char **argv);

    ~Config();
    // void setServers(const std::map<std::string, ServerConfig> servers);
    const ServerConfig& operator[](int index) const;
    void    parseConfig(const std::string filename);
    int getNumberOfServer() const;
};

void replaceTabsWithSpaces(std::string& str);
#endif