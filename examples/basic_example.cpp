#include "SimpleVideoInput.h"
#include <iostream>

int main(int argc, char *argv[])
{
	if (argc < 2) {
		std::cerr << "Usage: basic_example <videofile>" << std::endl;
		return 1;
	}
	SimpleVideoInput v(argv[1]);
		
	return 0;
}
