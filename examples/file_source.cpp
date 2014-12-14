#include "VideoSource.h"
#include <opencv2/highgui/highgui.hpp>
#include <iostream>

class FileSource: public svi::VideoSource
{
public:
	FileSource(const char *fileName)
		: VideoSource()
	{
		m_file = fopen(fileName, "r");
		assert (m_file != nullptr);
	}

	~FileSource()
	{
		fclose(m_file);
	}

	int read(uint8_t *buf, int buf_size)
	{
		return fread(buf, 1, buf_size, m_file);
	}

private:
	FILE *m_file;
};

int main(int argc, char *argv[])
{
	if (argc < 2) {
		std::cerr << "Usage: " << argv[0] << " <videofile>" << std::endl;
		return 1;
	}
	FileSource fs(argv[1]);
	svi::SimpleVideoInput v(fs);
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
