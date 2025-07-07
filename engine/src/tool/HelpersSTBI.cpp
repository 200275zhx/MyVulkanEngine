#include "tool/HelpersSTBI.hpp"
#include <iostream>

namespace mve {
	int GetNumChannel(const char* inputFilePath) {
		int numChannel;
		if (!stbi_info(inputFilePath, nullptr, nullptr, &numChannel)) {
			fprintf(stderr, "Failed to get image info %s\n", inputFilePath);
			return 0;
		}
		return numChannel;
	}
}