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
		std::cout << "Read Frame #" << count++ << "(" << v.millisecondsPerFrame() << ")" << std::endl;
#ifdef __APPLE__
		/* For OS X and C++11 I need clang's libc++,
		   but my OpenCV is linked against GCC's libstdc++
		   Thus I cannot pass std::string to CV and have to
		   use the old C-API as a fallback *sigh*
		 */
		IplImage tmp = image;
		cvShowImage("Current frame", &tmp);
#else
		cv::imshow("Current frame", image);
#endif
		if (cv::waitKey(v.millisecondsPerFrame()) == 'q')
			break;
	}
		
	return 0;
}
