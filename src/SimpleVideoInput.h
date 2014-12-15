#pragma once

#include "Decoder.h"

#include <opencv2/core/core.hpp>


namespace svi {

	class SimpleVideoInput: public Decoder
	{
	public:
		using Decoder::Decoder;

		SimpleVideoInput & operator>>(cv::Mat & image);
		bool read(cv::Mat & image);
		bool retrieve(cv::Mat & image);
	};

}