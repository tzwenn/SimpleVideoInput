#pragma once

#include "Decoder.h"

namespace svi {

	class VideoSource
	{
		enum { default_bufsize = 4096 };

	public:
		VideoSource(int buffer_size = default_bufsize);
		virtual ~VideoSource();

		virtual int read(uint8_t *buf, int buf_size) = 0;

	protected:
		AVIOContext *getAVIO() const
		{ return m_ioCtx; }

	private:
		AVIOContext *m_ioCtx;

		static int s_read(void *opaque, uint8_t *buf, int buf_size)
		{ return static_cast<VideoSource *>(opaque)->read(buf, buf_size); }

		friend Decoder::Decoder(const VideoSource &);
	};

} // namespace svi