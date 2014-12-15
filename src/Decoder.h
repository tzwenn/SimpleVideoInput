#pragma once

#include <string>

struct AVIOContext;

namespace svi {

	struct DecoderDetail;
	class VideoSource;

	class Decoder
	{
	public:
		Decoder();
		Decoder(const std::string & fileName);
		Decoder(const VideoSource & videoSource);
		virtual ~Decoder();

		bool open(const std::string & fileName);
		bool isOpened() const;
		void release();

		long millisecondsPerFrame() const;
		int videoWidth() const;
		int videoHeight() const;

		bool grab();
		bool readBGR24Frame(uint8_t *dst, int *linesize);

	private:
		DecoderDetail *m_detail;

		void initLibavcodec();
		void openFormatContext(const std::string & fileName, AVIOContext *ioCtx = nullptr);
		void findFirstVideoStream();
		void openCodec();
		void prepareTargetBuffer();
		void prepareResizeContext();
	};

} // namespace svi