#pragma once
#include <string>
class MoDetSettings {

	std::string m_device;
	std::string m_name;
	int m_video_before_motion;
	int m_video_after_motion;
	int m_min_motion;
	int m_cool_down;
	std::string m_script_launcher;
	std::string m_script;
	std::string m_script_dir;
	std::string m_output_dir;
	std::string m_enable_display;
	double m_fps;

	MoDetSettings();
	MoDetSettings(const MoDetSettings&) = delete;
	MoDetSettings(MoDetSettings&&) = delete;
	MoDetSettings& operator=(const MoDetSettings&) = delete;
	MoDetSettings& operator=(MoDetSettings&&) = delete;

	static MoDetSettings &instance() {
		static MoDetSettings modet_setting;
		return modet_setting;
	}

	void __print()const;

	bool __read(const std::string &_path);

public:
	static bool read(const std::string &_path) {
		return instance().__read(_path);
	}

	static std::string get_device() {
		return instance().m_device;
	}

	static std::string get_name() {
		return instance().m_name;
	}

	static int get_video_before_motion() {
		return instance().m_video_before_motion;
	}

	static int get_video_after_motion() {
		return instance().m_video_after_motion;
	}

	static int get_minimum_motion() {
		return instance().m_min_motion;
	}

	static int get_cool_down() {
		return instance().m_cool_down;
	}

	static std::string get_script_launcher() {
		return instance().m_script_launcher;
	}

	static std::string get_script() {
		return instance().m_script;
	}

	static std::string get_script_dir() {
		return instance().m_script_dir;
	}

	static std::string get_output_dir() {
		return instance().m_output_dir;
	}

	static std::string get_enable_display() {
		return instance().m_enable_display;
	}

	static double get_fps() {
		return instance().m_fps;
	}

	static void print() {
		instance().__print();
	}
	
};
