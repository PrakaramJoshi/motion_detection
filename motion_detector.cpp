#include "stdafx.h"
#include "motion_detector.h"
#include "logger.h"

motion_detector::motion_detector(BlockingQueue<cv::Mat> *_inq,
	const input_property *_property) {
	m_inq = _inq;
	m_input_property = _property;
}

void motion_detector::run() {
	cv::Mat *mat=nullptr;
	cv::Mat diffused_frame;
	cv::Mat diff_frame;
	cv::Mat prev_frame;
	cv::Mat diff_threshold;
	cv::Mat non_Zero;
	cv::Mat gray;
	bool recording = false;
	std::size_t cool_down = 0;
	m_v.init(m_input_property);
	while (m_inq->Remove(&mat)) {
		if (mat) {
			if (cool_down != 0) {
				cool_down -= 1;
				delete mat;
				mat = nullptr;
				continue;
			}
			/*for (auto c = 0; c < mat->cols; c++) {
				for (auto r = 0; r< mat->rows; r++) {
					auto v=mat->data[c + r*(mat->cols*3)]+ mat->data[c +1+ r*(mat->cols * 3)]+ mat->data[c+2 + r*(mat->cols * 3)];
					v = v / 3.0;
				}
			}*/
			cv::cvtColor(*mat, gray, cv::COLOR_RGB2GRAY);
			cv::GaussianBlur(gray,
				diffused_frame,
				cv::Size(5, 5),
				2.2);
			bool motion = false;
			if (prev_frame.size().area()!=0) {
				
				cv::subtract(prev_frame, diffused_frame, diff_frame);
				
				double area = diff_frame.size().area();
				cv::threshold(diff_frame, diff_threshold, 10, 255, cv::THRESH_TOZERO);
				
				cv::findNonZero(diff_threshold, non_Zero);
				double non_zero_area = non_Zero.size().area();
				double motion_percent = non_zero_area*100.0 / area;
				
#ifdef DEBUG_LOGS
				AceLogger::Log("motion : " + std::to_string(motion_percent));
#endif
				if (motion_percent> 2.0) {
					motion = true;
#ifdef DEBUG_LOGS
					AceLogger::Log(" Has motion   : " + std::string(m_v.has_motion() ? "Yes" : "No"));
					AceLogger::Log(" Motion count : " + std::to_string(m_v.get_motion_count()));
#endif
				}
			}
			diffused_frame.copyTo(prev_frame);
			if (recording) {
				m_v.motion_recording_frame(mat);
			}
			else {
				m_v.add_frame(mat, motion);
			}
			if (m_v.has_motion()) {
				if (!recording) {
					recording = true;
				}
			}
			if (m_v.is_motion_recording_full()) {
				m_v.save();
				recording = false;
				cool_down = m_v.get_cool_down_frame_count();
			}
		}
		mat = nullptr;
		
	}
}