#pragma once

struct SimpleVideoInputDetail;

class SimpleVideoInput 
{
public:
	SimpleVideoInput(const char *filename);
	virtual ~SimpleVideoInput();

private:
	SimpleVideoInputDetail *m_detail;

	void initLibavcodec();
};

