#include "SimpleVideoInput.h"

#include <memory>
#include <mutex>
#include <vector>

extern "C" { 
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
}

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
	std::shared_ptr<AVCodecContext> videoCodec;
	std::vector<uint8_t> codecContextExtraData;
};

static void initLibavcodec()
{
	static std::once_flag initAVFlag;
	std::call_once(initAVFlag, []() {
		av_register_all();
	});

}

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
	return false;
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
	
	m_detail->videoCodec = std::shared_ptr<AVCodecContext>(avcodec_alloc_context3(codec), [](AVCodecContext* c) { avcodec_close(c); av_free(c); });
	
	m_detail->codecContextExtraData = std::vector<uint8_t>(orgCodecCtx->extradata, orgCodecCtx->extradata + orgCodecCtx->extradata_size);
	m_detail->videoCodec->extradata = m_detail->codecContextExtraData.data();
	m_detail->videoCodec->extradata_size = orgCodecCtx->extradata_size;
	
	if (avcodec_open2(m_detail->videoCodec.get(), codec, nullptr) < 0)
		throw std::runtime_error("Could not open codec");
}