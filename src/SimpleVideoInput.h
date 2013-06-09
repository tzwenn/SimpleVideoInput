#pragma once

#include <opencv2/core/core.hpp>

struct SimpleVideoInputDetail;

class SimpleVideoInput 
{
public:
	SimpleVideoInput(const char *filename);
	virtual ~SimpleVideoInput();
	
	bool read(cv::Mat & image);

private:
	SimpleVideoInputDetail *m_detail;
	
	void openFirstVideoStream();
	void openCodec();
};

