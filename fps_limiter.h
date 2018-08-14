#ifndef FPS_LIMITER_H
#define FPS_LIMITER_H
#include <chrono>
#include <thread>

class fps_limiter {
	std::chrono::milliseconds m_delay;
	double m_frames_skip;
	double m_frames_skipped;
	double m_camera_fps;
	double m_target_fps;
	bool m_enable;
public:

	fps_limiter() {
		m_enable = false;
		m_delay = std::chrono::milliseconds(static_cast<int>(0));
		m_camera_fps = 0.0;
		m_target_fps = 0.0;
		m_frames_skip = 0;
		m_frames_skipped = 0.0;
	}

	void set_camera_fps(double _fps) {
		m_camera_fps = _fps;
	}

	void set_target_fps(double _fps) {
		m_target_fps = _fps;
	}

	double init() {
		if (m_target_fps < m_camera_fps) {
			m_enable = true;
			auto delta = m_camera_fps - m_target_fps;
			m_frames_skip = static_cast<int>(m_camera_fps/m_target_fps);
			auto delay = (1000.0 / m_camera_fps)*delta;
			using namespace std::chrono_literals;
			m_delay = std::chrono::milliseconds(static_cast<int>(delay));
			return m_target_fps;
		}
		return m_camera_fps;
	}

	void enforce() {
		if (m_enable)
			std::this_thread::sleep_for(m_delay);
	}

	bool use_frame() {
		if (m_enable) {
			if (m_frames_skipped < m_frames_skip) {
				m_frames_skipped++;
				return false;
			}
			m_frames_skipped = 0;
		}
		return true;
	}
};
#endif