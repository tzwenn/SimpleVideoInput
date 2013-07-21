#include "SimpleVideoInput.h"

#include <memory>
#include <mutex>
#include <vector>

extern "C" { 
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
}

struct SimpleVideoInputDetail
{
	std::shared_ptr<AVFormatContext> format;
	std::shared_ptr<AVCodecContext> codecCtx;
	std::shared_ptr<uint8_t> buffer;
	std::shared_ptr<AVFrame> currentFrame;
	std::shared_ptr<AVFrame> currentFrameBGR24;

	struct SwsContext *swsCtx;
	int videoStreamIdx;
	bool isOpened;
	int BGR24Linesize[AV_NUM_DATA_POINTERS];

	int videoWidth;
	int videoHeight;

	SimpleVideoInputDetail()
		: swsCtx(nullptr),
		  videoStreamIdx(-1),
		  isOpened(false),
		  videoWidth(0),
		  videoHeight(0)
	{}
};

/******************************************************************************/
/*                              SimpleVideoInput                              */


SimpleVideoInput::SimpleVideoInput()
	: m_detail(new SimpleVideoInputDetail)
{
}

SimpleVideoInput::SimpleVideoInput(const std::string & fileName)
	: m_detail(new SimpleVideoInputDetail)
{
	openFormatContext(fileName);
}

SimpleVideoInput::~SimpleVideoInput()
{
	delete m_detail;
}

bool SimpleVideoInput::open(const std::string & fileName)
{
	try {
		if (isOpened())
			release();
		openFormatContext(fileName);
		return true;
	} catch (...) {
		return false;
	}
}

void SimpleVideoInput::openFormatContext(const std::string & fileName)
{
	initLibavcodec();
	
	///////////////////
	// Open file
		
	AVFormatContext *pFormatCtx = nullptr;
	if (avformat_open_input(&pFormatCtx, fileName.c_str(), nullptr, nullptr) != 0)
		throw std::runtime_error("Cannot open file: Does not exists or is no supported format");

	m_detail->format
		= std::shared_ptr<AVFormatContext>(pFormatCtx,
										   [](AVFormatContext *format)
										   {
											   avformat_close_input(&format);
										   });
	if (avformat_find_stream_info(pFormatCtx, nullptr) < 0)
		throw std::runtime_error("File contains no streams");

	findFirstVideoStream();
	openCodec();
	prepareTargetBuffer();
	prepareResizeContext();

	m_detail->isOpened = true;
}

bool SimpleVideoInput::isOpened() const
{
	return m_detail->isOpened;
}

void SimpleVideoInput::release()
{
	std::unique_ptr<SimpleVideoInputDetail> oldDetail(m_detail);

	m_detail = new SimpleVideoInputDetail;
}

long SimpleVideoInput::millisecondsPerFrame() const
{
	double frame_delay = av_q2d(m_detail->codecCtx->time_base);
	return m_detail->codecCtx->ticks_per_frame * 1000 * frame_delay;
}

void SimpleVideoInput::initLibavcodec()
{
	static std::once_flag initAVFlag;
	std::call_once(initAVFlag, []() {
		av_register_all();
	});

}

void SimpleVideoInput::findFirstVideoStream()
{
	for (int streamIdx = 0; streamIdx < m_detail->format->nb_streams; ++streamIdx) {
		AVStream *stream = m_detail->format->streams[streamIdx];
		if (stream->codec->codec_type == AVMEDIA_TYPE_VIDEO) {
			m_detail->videoStreamIdx = streamIdx;
			return;
		}
	}

	throw std::runtime_error("Didn't find any video stream in the file (probably audio file)");
}

void SimpleVideoInput::openCodec()
{
	AVStream *stream = m_detail->format->streams[m_detail->videoStreamIdx];
	AVCodecContext *pCodecCtx = stream->codec;

	AVCodec *codec = avcodec_find_decoder(pCodecCtx->codec_id);
	if (!codec)
		throw std::runtime_error("Codec required by video file not available");

	if (avcodec_open2(pCodecCtx, codec, nullptr) < 0)
		throw std::runtime_error("Could not open codec");

	m_detail->codecCtx = std::shared_ptr<AVCodecContext>(pCodecCtx, &avcodec_close);
	m_detail->videoWidth = pCodecCtx->width;
	m_detail->videoHeight = pCodecCtx->height;
}

void SimpleVideoInput::prepareTargetBuffer()
{
	m_detail->currentFrame = std::shared_ptr<AVFrame>(avcodec_alloc_frame(), &av_free);
}

void SimpleVideoInput::prepareResizeContext()
{
	m_detail->swsCtx = sws_getContext(m_detail->videoWidth,
									  m_detail->videoHeight,
									  m_detail->codecCtx->pix_fmt,
									  m_detail->videoWidth,
									  m_detail->videoHeight,
									  PIX_FMT_BGR24,
									  SWS_BILINEAR,
									  nullptr, nullptr, nullptr);

	if (!m_detail->swsCtx)
		throw std::runtime_error("Cannot get sws_getContext for resizing");

	AVPicture frameAsBGR24;
	avpicture_fill(&frameAsBGR24, nullptr, PIX_FMT_BGR24, m_detail->videoWidth, m_detail->videoHeight);
	memcpy(m_detail->BGR24Linesize, frameAsBGR24.linesize, sizeof(m_detail->BGR24Linesize));
}

SimpleVideoInput & SimpleVideoInput::operator>>(cv::Mat & image)
{
	if (!read(image))
		image.release();

	return *this;
}

bool SimpleVideoInput::read(cv::Mat & image)
{
	return grab() && retrieve(image);
}

bool SimpleVideoInput::grab()
{
	AVPacket packet;
	int isFrameAvailable;

	while (av_read_frame(m_detail->format.get(), &packet) >= 0) {
		if (packet.stream_index != m_detail->videoStreamIdx)
			continue;

		avcodec_decode_video2(m_detail->codecCtx.get(), m_detail->currentFrame.get(), &isFrameAvailable, &packet);

		// ERROR, ERROR, FIXME!!!!!!!!!!!!!!!!
		// I wrongly assume a packet fills whole frame
		if (isFrameAvailable)
			return true;
	}

	return false;
}

bool SimpleVideoInput::retrieve(cv::Mat & image)
{
	image.create(m_detail->videoHeight, m_detail->videoWidth, CV_8UC3);

	uint8_t *const dst[1] = {image.data};
	sws_scale(m_detail->swsCtx,
			  (uint8_t const * const *)m_detail->currentFrame->data,
			  m_detail->currentFrame->linesize,
			  0,
			  m_detail->videoHeight,
			  dst,
			  m_detail->BGR24Linesize);

	image.step = m_detail->BGR24Linesize[0];

	return true;
}
