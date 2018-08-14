#ifndef VIDEO_DISPLAY
#define VIDEO_DISPLAY
#include "BlockingQueue.h"
#include "opencv_include.h"

class display {
	BlockingQueue<cv::Mat> * m_inq;
	std::string m_name;
	display() = delete;
	display(const display&) = delete;
	display(display &&) = delete;
	display& operator=(const display&) = delete;
	display& operator=(display&&) = delete;
public:
	display(BlockingQueue<cv::Mat> *_inq);
	void run();
};
#endif