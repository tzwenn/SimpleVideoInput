#include "SimpleVideoInput.h"
#include <opencv2/highgui/highgui.hpp>
#include <iostream>

int main(int argc, char *argv[])
{
	if (argc < 2) {
		std::cerr << "Usage: basic_example <videofile>" << std::endl;
		return 1;
	}
	svi::SimpleVideoInput v(argv[1]);
	cv::Mat image;
	int count = 0;
	while (v.read(image)) {
		std::cout << "Read Frame #" << count++ << " (" << v.millisecondsPerFrame() << "ms)" << std::endl;

		cv::imshow("Current frame", image);
		if (cv::waitKey(v.millisecondsPerFrame()) == 'q')
			break;
	}
		
	return 0;
}
