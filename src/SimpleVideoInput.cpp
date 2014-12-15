#include "SimpleVideoInput.h"

using namespace svi;

SimpleVideoInput & SimpleVideoInput::operator>>(cv::Mat & image)
{
	if (!read(image))
		image.release();

	return *this;
}

bool SimpleVideoInput::read(cv::Mat & image)
{
	return grab() && retrieve(image);
}

bool SimpleVideoInput::retrieve(cv::Mat & image)
{
	int step;
	image.create(videoHeight(), videoWidth(), CV_8UC3);
	bool res = readBGR24Frame(image.data, &step);
	image.step = step;

	return res;
}