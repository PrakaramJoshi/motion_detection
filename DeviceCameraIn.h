#pragma once
#include "BlockingQueue.h"
#include "input_property.h"
#include "opencv_include.h"
#include "fps_limiter.h"
#include <atomic>


class DeviceCapture {

	cv::VideoCapture *m_capture;

	fps_limiter m_fps_limiter;

	double m_fps;

	int m_width;

	int m_height;

	double determine_fps();

public:

	DeviceCapture();

	bool init_capture();

	void capture(BlockingQueue<cv::Mat>*_inq, std::atomic<bool> *_keep_alive);

	double get_fps()const;

	int get_width();

	int get_height();

	void clear();

	~DeviceCapture();
};