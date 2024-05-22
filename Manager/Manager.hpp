#ifndef MANAGER_HPP
# define MANAGER_HPP


#include "../request/NResponseUtils.hpp"
#include "../utils/utils.hpp"
#include "../request/Request.hpp"

class Manager {
	public:
		Manager();
		~Manager();
		static ResponseUtils responseUtils;
		static Utils utils;
		static RequestUtils requestUtils;
};

#endif