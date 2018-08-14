#include "stdafx.h"
#include "video_display.h"
#include "modet_settings.h"
display::display(BlockingQueue<cv::Mat> *_inq ){
	m_inq = _inq;
	m_name = MoDetSettings::get_name();
	cv::namedWindow(m_name.c_str(), CV_WINDOW_AUTOSIZE);
}

void display::run() {
	cv::Mat *mat = nullptr;
	
	while (m_inq->Remove(&mat)) {
		try {
			if (mat) {
				
				cv::imshow(m_name.c_str(), *mat);
				cv::waitKey(1);
			}
			delete mat;
			mat = nullptr;
		}
		catch (...)
		{
			
		}
	}
}