#include "SimpleVideoInput.h"
#include <opencv2/opencv.hpp>
#include <iostream>

int main(int argc, char *argv[])
{
	if (argc < 2) {
		std::cerr << "Usage: basic_example <videofile>" << std::endl;
		return 1;
	}
	SimpleVideoInput v(argv[1]);
	cv::Mat image;
	int count = 0;
	while (v.read(image)) {
		cv::imshow("Current frame", image);
		cv::waitKey(40);
		std::cout << "Read Frame " << count++ << "(" << v.millisecondsPerFrame() << ")" << std::endl;
	}
		
	return 0;
}
