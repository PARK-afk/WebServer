#include "LocationConfig.hpp"
#include "Config.hpp"

bool	LocationConfig::isCgi() const {
	return _cgi;
}

void	LocationConfig::setCgi(bool cgi) {
	_cgi = cgi;
}

LocationConfig::LocationConfig(std::ifstream &file, std::string &defaultIndex) : _autoindex(false), _defaultIndex(defaultIndex), _cgi(false) {
    std::string     line;
    std::string     key, value;

    while (std::getline(file, line)) {
        if (line[0] == '}') {
            file.seekg(-(line.length() + 1), std::ios::cur);
            break;
        }
        replaceTabsWithSpaces(line);
        std::istringstream iss(line);
        std::string key;
        
        if (!(iss >> key)) continue;
        
        if (key == "}") {
            break ;
        }
    
        if (key == "location") {
            std::string path;
            iss >> path;
            if (path[0] != '/') {
                while (iss >> path) {
                    if (path[0] == '\\') {
                        break;
                    }
                }
            }
            if (path.empty())
                continue;
            else
                this->_path = path;
        } else if (key == "alias") {
            std::string alias;
            iss >> alias;
            if (alias.empty())
                continue;
            else
                this->_alias = alias;
        } else if (key == "root") {
            std::string root;
            iss >> root;
            if (root.empty())
                continue;
            else
                this->_root = root;
        } else if (key == "index") {
            std::string index;
            iss >> index;
            if (index.empty())
                continue;
            else
                this->_index = index;
        } else if (key == "autoindex") {
            std::string autoindex;
            iss >> autoindex;
            if (autoindex == "on")
                this->_autoindex = true;
            else
                this->_autoindex = false;
        } else if (key == "allow_methods") {
            std::string allow_methods;
            while (iss >> allow_methods) {
                this->_allow_methods.push_back(allow_methods);
            }
        }
        else if (key == "return") {
            std::string return_url;
			std::string return_code;
			iss >> return_code;
            iss >> return_url;
            if (return_url.empty())
                continue;
            else
                this->_return_url = return_url;
				this->_return_code = return_code;
        } else if (key == "cgi_pass") {
			setCgi(true);
            std::string fastcgi_pass;
            iss >> fastcgi_pass;
            if (fastcgi_pass.empty())
                continue;
            else
                this->_fastcgi_pass = fastcgi_pass;
        } else if (key == "cgi_param") {
            std::string fastcgi_param;
            iss >> fastcgi_param;
            if (fastcgi_param.empty())
                continue;
            else
                this->_fastcgi_param = fastcgi_param;
        } else if (key == "upload_pass") {
            std::string upload_pass;
            iss >> upload_pass;
            if (upload_pass.empty())
                continue;
            else
                this->_upload_pass = upload_pass;
        } else if (key == "upload_store") {
            std::string upload_store;
            iss >> upload_store;
            if (upload_store.empty())
                continue;
            else
                this->_upload_store = upload_store;
        } else if (key == "upload_state_store") {
            std::string upload_state_store;
            iss >> upload_state_store;
            if (upload_state_store.empty())
                continue;
            else
                this->_upload_state_store = upload_state_store;
        } else if (key == "upload_resume") {
            std::string upload_resume;
            iss >> upload_resume;
            if (upload_resume.empty())
                continue;
            else
                this->_upload_resume = true;
        } else if (key == "upload_max_file_size") {
            std::string upload_max_file_size;
            iss >> upload_max_file_size;
            if (upload_max_file_size.empty())
                continue;
            else
                this->_upload_max_file_size = upload_max_file_size;
        } else if (key == "upload_limit_rate") {
            std::string upload_limit_rate;
            iss >> upload_limit_rate;
            if (upload_limit_rate.empty())
                continue;
            else
                this->_upload_limit_rate = upload_limit_rate;
        } else if (key == "upload_cleanup") {
            std::string upload_cleanup;
            iss >> upload_cleanup;
            if (upload_cleanup.empty())
                continue;
            else
                this->_upload_cleanup.push_back(std::atoi(upload_cleanup.c_str()));
        } else if (key == "try_files") {
            std::string try_files;
            iss >> try_files;
            if (try_files.empty())
                continue;
            else
                this->_try_files.push_back(try_files);
        } else if (key == "etag") {
            std::string etag;
            iss >> etag;
            if (etag == "on")
                this->_etag = true;
            else if (etag == "off")
                this->_etag = false;
        }
    }
}

std::string                 LocationConfig::getPath() const { return this->_path; }

std::string                 LocationConfig::getAlias() const { return _alias; }

std::string LocationConfig::getRoot() const {
    std::string root = _root;
    
    std::replace(root.begin(), root.end(), '\\', '/');
    std::string::size_type pos = 0;
    while ((pos = root.find("//", pos)) != std::string::npos) {
        root.erase(pos, 1);
    }
    
    if (!root.empty() && root.back() == '/') {
        root.pop_back();
    }
    
    return root;
}

std::string                 LocationConfig::getIndex() const {
    if (_index.empty())
        return _defaultIndex;
    return _index;
}

bool                        LocationConfig::getAutoindex() const {
    return _autoindex;
}

std::vector<std::string>    LocationConfig::getAllowMethods() const {
    return _allow_methods;
}

std::string            LocationConfig::getAllowedMethod(size_t i) const {
    if (i >= _allow_methods.size()) {return "";}
    else return _allow_methods[i];
}

std::string                 LocationConfig::getReturnCode() const {
    return _return_code;
}

std::string                 LocationConfig::getReturnUrl() const {
    return _return_url;
}

std::string                 LocationConfig::getFastcgiPass() const {
    return _fastcgi_pass;
}

std::string                 LocationConfig::getUploadPass() const {
    return _upload_pass;
}

std::string                 LocationConfig::getUploadStore() const {
    return _upload_store;
}

std::string                 LocationConfig::getUploadStateStore() const {
    return _upload_state_store;
}

bool                        LocationConfig::getUploadResume() const {
    return _upload_resume;
}

std::string                 LocationConfig::getUploadMaxFileSize() const {
    return _upload_max_file_size;
}

std::string                 LocationConfig::getUploadLimitRate() const {
    return _upload_limit_rate;
}

std::vector<int>            LocationConfig::getUploadCleanup() const {
    return _upload_cleanup;
}

std::vector<std::string>    LocationConfig::getTryFiles() const {
    return _try_files;
}

bool                        LocationConfig::isEtag() const {
    return _etag;
}