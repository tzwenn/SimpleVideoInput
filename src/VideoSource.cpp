#include "VideoSource.h"

#include <stdexcept>

extern "C" {
#include <libavformat/avio.h>
}

VideoSource::VideoSource(int buffer_size)
{
	m_initialBuffer = static_cast<unsigned char *>(av_malloc(buffer_size));

	m_ioCtx = avio_alloc_context(m_initialBuffer, buffer_size, 0, this,
								 &VideoSource::readMem,	nullptr, nullptr);
	if (!m_ioCtx)
		throw std::runtime_error("Cannot create AVIOContext");
}

VideoSource::~VideoSource()
{
	av_free(m_ioCtx->buffer);
	av_free(m_ioCtx);
}
