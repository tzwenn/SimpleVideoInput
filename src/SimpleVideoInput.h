#pragma once

#include <opencv2/core/core.hpp>

struct AVIOContext;

namespace svi {

	struct SimpleVideoInputDetail;
	class VideoSource;

	class SimpleVideoInput
	{
	public:
		SimpleVideoInput();
		SimpleVideoInput(const std::string & fileName);
		SimpleVideoInput(const VideoSource &videoSource);
		virtual ~SimpleVideoInput();

		bool open(const std::string & fileName);
		bool isOpened() const;
		void release();

		long millisecondsPerFrame() const;

		SimpleVideoInput & operator>>(cv::Mat & image);
		bool read(cv::Mat & image);
		bool grab();
		bool retrieve(cv::Mat & image);

	private:
		SimpleVideoInputDetail *m_detail;

		void initLibavcodec();
		void openFormatContext(const std::string & fileName, AVIOContext *ioCtx = nullptr);
		void findFirstVideoStream();
		void openCodec();
		void prepareTargetBuffer();
		void prepareResizeContext();
	};

} // namespace svi