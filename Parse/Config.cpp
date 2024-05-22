#include "Config.hpp"

void replaceTabsWithSpaces(std::string& str) {
    std::replace(str.begin(), str.end(), '\t', ' ');
}

bool validateLocationConfig(const std::string &line) {
    std::size_t locationPos = line.find("location");
    if (locationPos == std::string::npos) {
        return false;
    }

    std::size_t bracePos = line.find("{");
    if (bracePos == std::string::npos) {
        return false;
    }

    if (bracePos < locationPos) {
        return false;
    }

    return true;
}

bool Config::preConfigCheck(const std::string filename) {
    int locationCnt = 0;
    int serverCnt = 0;
    std::string line;
    bool serverBlock = false;
    bool locationBlock = false;
    int indentLevel = 0;

    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "\033[31mError: Could not open file " << filename << "\033[0m" << std::endl;
        exit(1);
    }

    std::cout << "====================\n";
    std::cout << "Test Start\n";
    std::cout << "====================\n\n";

    while (getline(file, line)) {
        replaceTabsWithSpaces(line);
        if (line.empty()) continue;

        std::istringstream iss(line);
        std::string key;
        iss >> key;

        if (key == "location" && serverBlock == true) {
            if (!validateLocationConfig(line)) {
                std::cerr << "\033[31mError: Invalid location block\033[0m" << std::endl;
                exit(1);
            }
            if (locationBlock == true) {
                std::cerr << "\n\033[31mError: Nested location block is not allowed\033[0m" << std::endl;
                exit(1);
            }
            locationCnt++;
            locationBlock = true;
            indentLevel++;
            std::cout << std::string(indentLevel, '\t') << "\033[32m==================== Location block " << locationCnt << " opened ====================\033[0m" << std::endl;
        }

        if (key == "}" && serverBlock && !locationBlock) {
            indentLevel--;
            std::cout << std::string(indentLevel, '\t') << "}" << std::endl;
            std::cout << std::string(indentLevel, '\t') << "\033[32m==================== Server block " << serverCnt << " closed ====================\033[0m\n" <<std::endl;
            serverBlock = false;
            locationCnt = 0;
        } else if (key == "}" && serverBlock && locationBlock) {
            indentLevel--;
            std::cout << std::string(indentLevel, '\t') << "\t}" << std::endl;
            std::cout << std::string(indentLevel, '\t') << "\t\033[32m==================== Location block " << locationCnt << " closed ====================\033[0m\n" << std::endl;
            locationBlock = false;
        } else if (serverBlock || locationBlock)
            std::cout << std::string(indentLevel, '\t') << line << std::endl;

        if (key == "server") {
            serverBlock = true;
            serverCnt++;
            std::cout << "\n\033[32m==================== Server block " << serverCnt << " opened ====================\033[0m" << std::endl;;
            std::cout << line << std::endl;
            indentLevel++;
        }
    }

    file.close();

    if (serverBlock || locationBlock) {
        std::cerr << "\033[31mError: Missing closing bracket '}'\033[0m" << std::endl;
        return false;
    }

    std::cout << "\033[34m====================\n\033[0m";
    std::cout << "\033[34m     Test End\n\033[0m";
    std::cout << "\033[34m====================\n\033[0m" << std::endl;

    return true;
}

void    Config::parseConfig(const std::string filename)
{
    std::string line;
    std::ifstream file(filename);

    if (!file.is_open() || preConfigCheck(filename) == false)
    {
        std::cerr << "Error: Could not open file " << filename << std::endl;
        exit(1);
    }

    while (getline(file, line))
    {
        if (line.empty())
            continue;
        std::istringstream iss(line);
        if (line.find("server") != std::string::npos && line[0] != '#')
        {
            file.seekg(-(line.length() + 1), std::ios::cur);
            ServerConfig server(file);
            _servers.push_back(server);
        }
    }
    file.close();
}

void	Config::setServerName(std::string serverName)
{
	_serverNmae = serverName;
}

Config::Config(int argc, char **argv) : _servers() {
    if (argc != 2) {
        std::cerr << "Error: Invalid number of arguments" << std::endl;
        parseConfig("default.conf");
    } else 
        parseConfig(argv[1]);
}

Config::~Config() {}

ServerConfig Config::operator[](Port port) const {
    for (size_t i = 0; i < _servers.size(); ++i) {
        if (_servers[i].getPortName() == port && _servers[i].getServerName().empty())
            return _servers[i];
    }

	for (size_t i = 0; i < _servers.size(); ++i) {
		if (_servers[i].getPortName() == port)
			return _servers[i];
	}
    throw std::runtime_error("This port doesn't match with any server");
}

ServerConfig Config::getServerConfig(Port port, const std::string &serverName) const {
	for (size_t i = 0; i < _servers.size(); ++i) {
		if (_servers[i].getPortName() == port && _servers[i].getServerName() == serverName)
			return _servers[i];
	}
	return this->operator[](port);
}

ServerConfig Config::operator[](int i) const {
    return _servers[i];
}

int Config::getNumberOfServer() const {
    return _servers.size();
}