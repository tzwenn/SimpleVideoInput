#include "SimpleVideoInput.h"

#include <memory>
#include <mutex>
#include <vector>

extern "C" { 
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
}

/******************************************************************************/
/*                                 Aux types                                  */

struct VideoPacket {
	
	explicit VideoPacket(AVFormatContext * ctxt = nullptr)
	{
		av_init_packet(&packet);
		packet.data = nullptr;
		if (ctxt) reset(ctxt);
	}
	
	VideoPacket(VideoPacket && other)
		: packet(std::move(other.packet))
	{
		other.packet.data = nullptr;
	}
	
	~VideoPacket()
	{
		if (packet.data)
			av_free_packet(&packet);
	}
	
	void reset(AVFormatContext * ctxt)
	{
		if (packet.data)
			av_free_packet(&packet);
		if (av_read_frame(ctxt, &packet) < 0)
			packet.data = nullptr;
	}
	
	AVPacket packet;
};


struct SimpleVideoInputDetail
{
	std::shared_ptr<AVFormatContext> format;
	AVStream *videoStream;
	std::shared_ptr<AVCodecContext> video;
	std::vector<uint8_t> codecContextExtraData;
	size_t offsetInData;

	SimpleVideoInputDetail()
	{
		videoStream = nullptr;
		offsetInData = 0;
	}

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
	
	AVFormatContext *avFormatPtr = nullptr;
	if (avformat_open_input(&avFormatPtr, filename, nullptr, nullptr) != 0) {
		throw std::runtime_error("Error while calling avformat_open_input (probably invalid file format)");
	}

	m_detail->format = std::shared_ptr<AVFormatContext>(avFormatPtr, &avformat_free_context);	
	if (avformat_find_stream_info(avFormatPtr, nullptr) < 0) {
		throw std::runtime_error("Error while calling avformat_find_stream_info");
	}

	openFirstVideoStream();
	openCodec();
}


SimpleVideoInput::~SimpleVideoInput()
{
	delete m_detail;
}

bool SimpleVideoInput::read(cv::Mat & image)
{
	std::shared_ptr<AVFrame> avFrame(avcodec_alloc_frame(), &av_free);
	VideoPacket packet;

	// Jump to next packet of our frame
	if (m_detail->offsetInData >= packet.packet.size) {
		do {
			packet.reset(m_detail->format.get());
		} while (packet.packet.stream_index != m_detail->videoStream->index);
	}
	
	AVPacket packetToSend;
	packetToSend.data = packet.packet.data + m_detail->offsetInData;
	packetToSend.size = packet.packet.size - m_detail->offsetInData;

	int isFrameAvailable = 0;
	const int processedLength = avcodec_decode_video2(m_detail->video.get(), avFrame.get(), &isFrameAvailable, &packetToSend);
	if (processedLength < 0) {
		av_free_packet(&packetToSend);
		throw std::runtime_error("Error while processing the data");
	}

	m_detail->offsetInData += processedLength;
	///////////////////////////////////////////////////////////////////////////

#if 0
	std::shared_ptr<AVPicture> pic = std::shared_ptr<AVPicture>(new AVPicture, [](AVPicture *pic) { avpicture_free(pic); });
	avpicture_alloc(pic.get(), PIX_FMT_RGB24, avFrame->width, avFrame->height);
	auto ctxt = sws_getContext(avFrame->width, avFrame->height, static_cast<PixelFormat>(avFrame->format), avFrame->width, avFrame->height, PIX_FMT_RGB24, SWS_BILINEAR, nullptr, nullptr, nullptr);
	if (ctxt == nullptr)
		throw std::runtime_error("Error while calling sws_getContext");
	sws_scale(ctxt, avFrame->data, avFrame->linesize, 0, avFrame->height, pic->data, pic->linesize);

	image = cv::Mat(avFrame->height, avFrame->width, CV_8UC3, pic->data);
#endif

	///////////////////////////////////////////////////////////////////////////
	return isFrameAvailable;
}

long SimpleVideoInput::millisecondsPerFrame() const
{
	return m_detail->video->ticks_per_frame * 1000 * m_detail->video->time_base.num / m_detail->video->time_base.den;
}

void SimpleVideoInput::openFirstVideoStream()
{
	for (int streamIdx = 0; streamIdx < m_detail->format->nb_streams; ++streamIdx) {
		AVStream *stream = m_detail->format->streams[streamIdx];
		if (stream->codec->codec_type == AVMEDIA_TYPE_VIDEO) {
			m_detail->videoStream = stream;
			return;
		}
	}

	throw std::runtime_error("Didn't find any video stream in the file (probably audio file)");
}

void SimpleVideoInput::openCodec()
{
	AVCodecContext *orgCodecCtx = m_detail->videoStream->codec;
	AVCodec *codec = avcodec_find_decoder(orgCodecCtx->codec_id);
	if (!codec)
		throw std::runtime_error("Codec required by video file not available");
	
	m_detail->video = std::shared_ptr<AVCodecContext>(avcodec_alloc_context3(codec), [](AVCodecContext* c) { avcodec_close(c); av_free(c); });
	
	m_detail->codecContextExtraData = std::vector<uint8_t>(orgCodecCtx->extradata, orgCodecCtx->extradata + orgCodecCtx->extradata_size);
	m_detail->video->extradata = m_detail->codecContextExtraData.data();
	m_detail->video->extradata_size = orgCodecCtx->extradata_size;
	
	if (avcodec_open2(m_detail->video.get(), codec, nullptr) < 0)
		throw std::runtime_error("Could not open codec");
}
