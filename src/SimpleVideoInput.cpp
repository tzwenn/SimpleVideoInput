#include "SimpleVideoInput.h"

#include <memory>
#include <mutex>
#include <vector>

extern "C" { 
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
}

#define DEBUG_DUMP

struct SimpleVideoInputDetail
{
	std::shared_ptr<AVFormatContext> format;
	std::shared_ptr<AVCodecContext> codecCtx;
	std::shared_ptr<uint8_t> buffer;
	std::shared_ptr<AVFrame> currentFrame;
	std::shared_ptr<AVFrame> currentFrameBGR24;

	AVStream *videoStream;
	struct SwsContext *swsCtx;
	int videoStreamIdx;
};

/******************************************************************************/
/*                                Aux methods                                 */

static void initLibavcodec()
{
	static std::once_flag initAVFlag;
	std::call_once(initAVFlag, []() {
		av_register_all();
	});

}

/******************************************************************************/
/*                              SimpleVideoInput                              */


SimpleVideoInput::SimpleVideoInput(const char *filename)
	: m_detail(new SimpleVideoInputDetail)
{
	initLibavcodec();
	
	///////////////////
	// Open file
		
	AVFormatContext *pFormatCtx = nullptr;
	if (avformat_open_input(&pFormatCtx, filename, nullptr, nullptr) != 0)
		throw std::runtime_error("Error while calling avformat_open_input (probably invalid file format)");

	m_detail->format = std::shared_ptr<AVFormatContext>(pFormatCtx, [](AVFormatContext *format)
														{
															avformat_close_input(&format);
														});
	if (avformat_find_stream_info(pFormatCtx, nullptr) < 0)
		throw std::runtime_error("Error while calling avformat_find_stream_info");

#ifdef DEBUG_DUMP
	av_dump_format(pFormatCtx, 0, filename, 0);
#endif

	findFirstVideoStream();
	openCodec();
	prepareTargetBuffers();
}

SimpleVideoInput::~SimpleVideoInput()
{
	delete m_detail;
}

long SimpleVideoInput::millisecondsPerFrame() const
{
	double frame_delay = av_q2d(m_detail->codecCtx->time_base);
	return m_detail->codecCtx->ticks_per_frame * 1000 * frame_delay;
}

void SimpleVideoInput::findFirstVideoStream()
{
	for (int streamIdx = 0; streamIdx < m_detail->format->nb_streams; ++streamIdx) {
		AVStream *stream = m_detail->format->streams[streamIdx];
		if (stream->codec->codec_type == AVMEDIA_TYPE_VIDEO) {
			m_detail->videoStream = stream;
			m_detail->videoStreamIdx = streamIdx;
			return;
		}
	}

	throw std::runtime_error("Didn't find any video stream in the file (probably audio file)");
}

void SimpleVideoInput::openCodec()
{
	AVCodecContext *pCodecCtx = m_detail->videoStream->codec;

	AVCodec *codec = avcodec_find_decoder(pCodecCtx->codec_id);
	if (!codec)
		throw std::runtime_error("Codec required by video file not available");

	if (avcodec_open2(pCodecCtx, codec, nullptr) < 0)
		throw std::runtime_error("Could not open codec");

	m_detail->codecCtx = std::shared_ptr<AVCodecContext>(pCodecCtx, &avcodec_close);
}

void SimpleVideoInput::prepareTargetBuffers()
{
	int videoWidth = m_detail->codecCtx->width;
	int videoHeight = m_detail->codecCtx->height;

	m_detail->currentFrame = std::shared_ptr<AVFrame>(avcodec_alloc_frame(), &av_free);
	m_detail->currentFrameBGR24 = std::shared_ptr<AVFrame>(avcodec_alloc_frame(), &av_free);

	int numBytes = avpicture_get_size(PIX_FMT_BGR24, videoWidth, videoHeight);
	uint8_t *buffer = (uint8_t *) av_malloc(numBytes * sizeof(uint8_t));
	m_detail->buffer = std::shared_ptr<uint8_t>(buffer, &av_free);

	m_detail->swsCtx = sws_getContext(videoWidth,
									  videoHeight,
									  m_detail->codecCtx->pix_fmt,
									  videoWidth,
									  videoHeight,
									  PIX_FMT_BGR24,
									  SWS_BILINEAR,
									  nullptr, nullptr, nullptr);

	if (!m_detail->swsCtx)
		throw std::runtime_error("Error while calling sws_getContext");

	avpicture_fill((AVPicture *)m_detail->currentFrameBGR24.get(), m_detail->buffer.get(), PIX_FMT_BGR24, videoWidth, videoHeight);
}


bool SimpleVideoInput::read(cv::Mat & image)
{
	AVPacket packet;
	int isFrameAvailable;

	while (av_read_frame(m_detail->format.get(), &packet) >= 0) {
		if (packet.stream_index != m_detail->videoStreamIdx)
			continue;

		avcodec_decode_video2(m_detail->codecCtx.get(), m_detail->currentFrame.get(), &isFrameAvailable, &packet);

		// ERROR, ERROR, FIXME!!!!!!!!!!!!!!!!
		// I wrongly assume a packet fills whole frame
		if (!isFrameAvailable)
			continue;

		sws_scale(m_detail->swsCtx,
				  (uint8_t const * const *)m_detail->currentFrame->data,
				  m_detail->currentFrame->linesize,
				  0,
				  m_detail->codecCtx->height,
				  m_detail->currentFrameBGR24->data,
				  m_detail->currentFrameBGR24->linesize);

		fillMat(image);

		return true;
	}

	return false;
}

void SimpleVideoInput::fillMat(cv::Mat & image)
{
	int width = m_detail->codecCtx->width;
	int height = m_detail->codecCtx->height;
	image.create(height, width, CV_8UC3);

	uint8_t *data = m_detail->currentFrameBGR24->data[0];
	int linesize = m_detail->currentFrameBGR24->linesize[0];

	for (int row = 0; row < height; ++row)
		memcpy(image.data + image.step * row, data + linesize * row, linesize);
}

