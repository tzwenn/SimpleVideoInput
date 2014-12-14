#pragma once

#include "SimpleVideoInput.h"

namespace svi {

	class VideoSource
	{
	public:
		VideoSource(int buffer_size = 4096);
		virtual ~VideoSource();

		virtual int read(uint8_t *buf, int buf_size) = 0;

	protected:
		AVIOContext *getAVIO() const
		{ return m_ioCtx; }

	private:
		AVIOContext *m_ioCtx;

		static int readMem(void *opaque, uint8_t *buf, int buf_size)
		{ return static_cast<VideoSource *>(opaque)->read(buf, buf_size); }

		friend SimpleVideoInput::SimpleVideoInput(const VideoSource &);
	};

} // namespace svi