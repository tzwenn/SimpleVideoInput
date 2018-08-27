#include "SimpleVideoInput.h"

#include <opencv2/highgui/highgui.hpp>
#include <iostream>

#define MS_PER_FRAME_MINI  30


int main(int argc, char *argv[])
{
	if (argc < 2) {
		std::cerr << "Usage: basic_example <videofile>" << std::endl;
		return 1;
	}
	svi::SimpleVideoInput v(argv[1]);
	cv::Mat image;
	int count = 0;
        cv::namedWindow("SimpleVideoInput", CV_WINDOW_NORMAL|CV_WINDOW_OPENGL);
        cv::resizeWindow("SimpleVideoInput", 1280, 720);
        int minTimeValue = MS_PER_FRAME_MINI;

	while (v.read(image)) {

                if (v.millisecondsPerFrame() > 0)
                    minTimeValue = v.millisecondsPerFrame();
                else
                    minTimeValue = MS_PER_FRAME_MINI;

		std::cout << "Read Frame #" << count++ << " (" << minTimeValue << "ms)" << std::endl;

		cv::imshow("SimpleVideoInput", image);
		if (cv::waitKey(minTimeValue) == 'q')
			break;
	}
		
	return 0;
}
