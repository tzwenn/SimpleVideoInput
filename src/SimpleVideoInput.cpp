#include "SimpleVideoInput.h"
#include <mutex>

extern "C" { 
#include <libavcodec/avcodec.h>
}

struct SimpleVideoInputDetail
{
	
};

SimpleVideoInput::SimpleVideoInput(const char *filename)
	: m_detail(new SimpleVideoInputDetail)
{
	initLibavcodec();
}


SimpleVideoInput::~SimpleVideoInput()
{
	delete m_detail;
}

void SimpleVideoInput::initLibavcodec()
{
	static std::once_flag initAVFlag;
	std::call_once(initAVFlag, []() 
		{
			avcodec_register_all();
		});
}

