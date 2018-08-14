#pragma once
#include "BlockingQueue.h"
#include "opencv_include.h"
#include "input_property.h"
#include <list>
#include <functional>
#include "modet_settings.h"
#include "logger.h"
#include "video_write.h"
struct video_buffer {

	struct video_data {
		std::vector<cv::Mat*> p_video_data;
		std::string p_dir;
		std::string p_file_name;
	};
	std::list<std::pair<cv::Mat*, bool >> m_v,m_motion_recording;
	std::size_t m_mcount;
	double m_min_motion_frames;
	double m_cool_down;
	double m_min_clip_frames_before;
	double m_min_clip_frames_after;
	const input_property *m_property;
	std::thread *m_video_save;
	BlockingQueue<video_data> m_video_data;


	video_buffer() {
		m_mcount = 0;
		m_video_save = nullptr;
	}

	void init(const input_property *_property) {
		m_property = _property;
		auto fps = _property->get_fps();
		m_min_motion_frames = MoDetSettings::get_minimum_motion() * fps;
		m_min_clip_frames_before = MoDetSettings::get_video_before_motion()*fps;
		m_min_clip_frames_after = MoDetSettings::get_video_after_motion()*fps;
		m_cool_down= MoDetSettings::get_cool_down()*fps;
		m_video_save = new std::thread(std::bind(&video_buffer::_video_save_and_run_script, this));
	}

	std::size_t get_cool_down_frame_count()const {
		return static_cast<std::size_t>(m_cool_down);
	}

	void add_frame(cv::Mat *_mat, bool _motion) {
		while (m_v.size() >= m_min_clip_frames_before) {
			delete m_v.front().first;
			if (m_v.front().second) {
				m_mcount -= 1;
			}
			m_v.pop_front();
		}
		m_v.push_back(std::make_pair(_mat, _motion));
		if (_motion) {
			m_mcount += 1;
		}
	}

	void motion_recording_frame(cv::Mat *_mat) {
		m_motion_recording.push_back(std::make_pair(_mat, true));
	}

	void clean() {
		for (auto v : m_v) {
			delete v.first;
		}
		for (auto v : m_motion_recording) {
			delete v.first;
		}
		m_motion_recording.clear();
		m_v.clear();
		m_mcount = 0;
	}

	bool has_motion()const {
		return (m_mcount > m_min_motion_frames)&& m_v.size()>(m_min_clip_frames_before*0.9);
	}

	bool is_motion_recording_full()const {
		return m_motion_recording.size() >  m_min_clip_frames_after;
	}

	std::size_t get_motion_count()const {
		return m_mcount;
	}
	void _video_save_and_run_script() {
		video_data *_data = nullptr;
		while (m_video_data.Remove(&_data)) {

			auto motion_video = _data->p_dir + _data->p_file_name + ".avi";
			video_write_opencv(motion_video, _data->p_video_data, m_property);
			std::for_each(_data->p_video_data.begin(), _data->p_video_data.end(),
				[](cv::Mat *_f) { delete _f; });
#ifdef WIN32
			std::string dir_delim = "\\";
#else
			std::string dir_delim = "/";
#endif
			std::string cmd = MoDetSettings::get_script_launcher() + " " + MoDetSettings::get_script_dir() + dir_delim + MoDetSettings::get_script() + " " + MoDetSettings::get_name() + " " + MoDetSettings::get_script_dir() + " " + motion_video;
			AceLogger::Log("running script " + cmd);
			std::system(cmd.c_str());
			delete _data;
		}
		
	}
	void save(bool _clean=true) {
		
		video_data *data = new video_data();
		std::for_each(m_v.begin(), m_v.end(), 
			[&data](const std::pair<cv::Mat*, bool> &_f) {cv::Mat *mat = new cv::Mat(); _f.first->copyTo(*mat); data->p_video_data.push_back(mat); });
		std::for_each(m_motion_recording.begin(), m_motion_recording.end(),
			[&data](const std::pair<cv::Mat*, bool> &_f) {cv::Mat *mat = new cv::Mat(); _f.first->copyTo(*mat); data->p_video_data.push_back(mat); });
#ifdef WIN32
		std::string dir_delim = "\\";
#else
		std::string dir_delim = "/";
#endif
		data->p_dir = MoDetSettings::get_output_dir() + dir_delim ;
		data->p_file_name = AceLogger::time_stamp_file_name() + "_motion_" + MoDetSettings::get_name();
		m_video_data.Insert(data);
		if (_clean) {
			clean();
		}
	}

	~video_buffer() {
		clean();
		m_video_data.ShutDown();
		if (m_video_save) {
			m_video_save->join();
			delete m_video_save;
			m_video_save = nullptr;
		}
	}
};

class motion_detector {
	BlockingQueue<cv::Mat> * m_inq;
	video_buffer m_v;
	const input_property *m_input_property;

	motion_detector()=delete;
	motion_detector(const motion_detector&) = delete;
	motion_detector(motion_detector &&) = delete;
	motion_detector& operator=(const motion_detector&) = delete;
	motion_detector& operator=(motion_detector&&) = delete;

public:
	motion_detector(BlockingQueue<cv::Mat> *_inq,
		const input_property *_property);
	void run();
};