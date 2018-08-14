#include "stdafx.h"
#include "DeviceCameraIn.h"
#include "modet_settings.h"
#include "logger.h"
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <sstream>

using namespace AceLogger;
using namespace cv;

DeviceCapture::DeviceCapture() {
	m_capture= nullptr;
}

bool DeviceCapture::init_capture() {

	m_capture = new cv::VideoCapture(CV_CAP_ANY); //0=default, -1=any camera, 1..99=your camera

	if (!m_capture)
	{
		Log("No camera detected", LOG_ERROR);
		return false;
	}
	cv::Mat mat;
	while(!m_capture->read(mat));
	m_width = mat.cols;
	m_height = mat.rows;

	AceLogger::Log("Calculating fps");
	std::vector<double> fpss;
	for (int i = 0; i < 5; i++) {
		auto fps = determine_fps();
		AceLogger::Log("Fps for test " + std::to_string(i + 1) + " : " + std::to_string(fps));
		if (fps > 0) {
			fpss.push_back(fps);
		}
	}
	double average_fps = 0.0;
	if (fpss.size() > 0) {
		auto sum = std::accumulate(fpss.begin(), fpss.end(), 0.0);
		average_fps = sum / static_cast<double>(fpss.size());
	}
	m_fps = average_fps;
	AceLogger::Log("calculated fps " + std::to_string(m_fps));
	if (m_fps > 1000.0 || m_fps <= 0.0) {
		return false;
	}
	m_fps_limiter.set_target_fps(MoDetSettings::get_fps());
	m_fps_limiter.set_camera_fps(m_fps);
	m_fps=m_fps_limiter.init();
	
	return true;
}

double DeviceCapture::determine_fps() {
	std::size_t max_retry_count = 10;
	double frame_count = 0;
	auto start = std::chrono::steady_clock::now();
	auto stop_time = std::chrono::steady_clock::now();
	auto delay = static_cast<double>(std::chrono::duration_cast<std::chrono::seconds>(stop_time - start).count());
	cv::Mat mat;
	while (frame_count<10|| delay<3) {
		if (m_capture->read(mat)) {
			frame_count++;
		}
		else {
			max_retry_count--;
			if (max_retry_count == 0) {
				break;
			}
		}
		stop_time = std::chrono::steady_clock::now();
		delay = static_cast<double>(std::chrono::duration_cast<std::chrono::seconds>(stop_time - start).count());
	}
	if (frame_count == 0.0) {
		return -1.0;
	}
	if (delay == 0.0) {
		return -1.0;
	}
	return frame_count / delay;
}

void DeviceCapture::capture(BlockingQueue<cv::Mat>*_inq, std::atomic<bool> *_keep_alive) {
	std::size_t max_retry_count = 10;
	while ((*_keep_alive)) {
		cv::Mat *mat = new cv::Mat();
		if (!m_capture->read(*mat)) {
			delete mat;
			max_retry_count--;
			if (max_retry_count>0)
				continue;
			else {
				AceLogger::LogErr("Max retry timeout.");
				clear();
				break;
			}
		}
		_inq->Insert(mat);
		m_fps_limiter.enforce();
	}
}


int DeviceCapture::get_width() {
	return m_width;
}

int DeviceCapture::get_height() {
	return m_height;
}

double DeviceCapture::get_fps()const {
	return m_fps;
}

void DeviceCapture::clear() {
	m_capture->release();
	delete m_capture;
	m_capture = nullptr;
}

DeviceCapture::~DeviceCapture() {
	clear();
}
