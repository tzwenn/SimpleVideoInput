#include "VideoSource.h"
#include <opencv2/highgui/highgui.hpp>
#include <iostream>

extern "C" {
#include <curl/curl.h>
}

class WebSource: public svi::VideoSource
{
public:
	WebSource(const char *url) :
		VideoSource(32768),
		m_currentBlock(0)
	{
		curl_global_init(CURL_GLOBAL_ALL);
		m_curl = curl_easy_init();
		assert (m_curl != nullptr);
		curl_easy_setopt(m_curl, CURLOPT_URL, url);
		curl_easy_setopt(m_curl, CURLOPT_WRITEFUNCTION, curlWrite);
		curl_easy_setopt(m_curl, CURLOPT_WRITEDATA, this);
	}

	~WebSource()
	{
		curl_easy_cleanup(m_curl);
		curl_global_cleanup();
	}

	int read(uint8_t *buf, int buf_size)
	{
		char range[80];

		m_targetBuf = buf;
		m_posInBuf = 0;
		m_missing = buf_size;
		snprintf(range, 80, "%d-%d", m_currentBlock, m_currentBlock + buf_size - 1);
		curl_easy_setopt(m_curl, CURLOPT_RANGE, range);
		CURLcode res = curl_easy_perform(m_curl);
		if (res != CURLE_OK && res != CURLE_WRITE_ERROR) {
			throw std::runtime_error(curl_easy_strerror(res));
		}
		m_currentBlock += m_posInBuf;
		return m_posInBuf;
	}

private:
	CURL *m_curl;
	int m_currentBlock;
	size_t m_posInBuf, m_missing;
	uint8_t *m_targetBuf;

	static size_t curlWrite(void *contents, size_t size, size_t nmemb, void *userp)
	{
		WebSource *t = static_cast<WebSource *>(userp);
		size_t realSize = std::min(size * nmemb, t->m_missing);

		memcpy(t->m_targetBuf + t->m_posInBuf, contents, realSize);
		t->m_posInBuf += realSize;
		t->m_missing -= realSize;

		return realSize;
	}
};

int main(int argc, char *argv[])
{
	if (argc < 2) {
		std::cerr << "Usage: " << argv[0] << " <url>" << std::endl;
		return 1;
	}

	WebSource ws(argv[1]);
	svi::SimpleVideoInput v(ws);
	cv::Mat image;
	int count = 0;
	while (v.read(image)) {
		std::cout << "Read Frame #" << count++ << " (" << v.millisecondsPerFrame() << "ms)" << std::endl;

		cv::imshow("Current frame", image);
		if (cv::waitKey(v.millisecondsPerFrame()) == 'q')
			break;
	}

	return 0;
}
