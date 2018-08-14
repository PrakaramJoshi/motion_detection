#pragma once
#include "opencv_include.h"
#include <mutex>
#include <thread>
class input_property {
	double m_fps;
	cv::Size m_size;
	mutable std::mutex m_mutex;
public:
	void set_fps(const double &_fps) {
		std::unique_lock<std::mutex>lock(m_mutex);
		m_fps = _fps;
	}

	void set_size(const cv::Size &_size) {
		std::unique_lock<std::mutex>lock(m_mutex);
		m_size = _size;
	}

	double get_fps()const {
		std::unique_lock<std::mutex>lock(m_mutex);
		return m_fps;
	}

	cv::Size get_size()const {
		std::unique_lock<std::mutex>lock(m_mutex);
		return m_size;
	}
};
