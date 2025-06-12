#include "core/application.hpp"
#include <iostream>

namespace mve {
	void mve::App::print(const std::string& msg)
	{
		std::cout << msg << std::endl;
	}
}