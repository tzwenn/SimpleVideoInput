#include "VideoSource.h"

#include <stdexcept>

extern "C" {
#include <libavformat/avio.h>
}

VideoSource::VideoSource(int buffer_size)
{
	unsigned char *initialBuf = static_cast<unsigned char *>(av_malloc(buffer_size));
	if (!initialBuf) {
		throw std::runtime_error("Cannot create initial buffer for AVIOContext");
	}

	m_ioCtx = avio_alloc_context(initialBuf, buffer_size, 0, this,
								 &VideoSource::readMem,	nullptr, nullptr);
	if (!m_ioCtx) {
		av_free(initialBuf);
		throw std::runtime_error("Cannot create AVIOContext");
	}
}

VideoSource::~VideoSource()
{
	av_free(m_ioCtx->buffer);
	av_free(m_ioCtx);
}
