#include "stdafx.h"
#include "modet_settings.h"
#include "app_settings.h"
#include "logger.h"

MoDetSettings::MoDetSettings() {
	m_device = "";
	m_name = "";
	m_video_before_motion = 0;
	m_video_after_motion = 0;
	m_min_motion = 0;
	m_cool_down = 0;
	m_script_launcher = "";
	m_script = "";
	m_script_dir = "";
	m_output_dir = "";
	m_enable_display = "";
	m_fps = 0.0;
}

bool MoDetSettings::__read(const std::string &_path) {
	INIReader reader(_path);

	if (reader.ParseError() < 0) {
		AceLogger::LogErr("Can't load " + _path);
		return false;
	}

	m_device = reader.Get("link", "device", "UNKNOWN");
	m_name = reader.Get("link", "name", "UNKNOWN");
	m_video_before_motion = reader.GetInteger("motion_detection", "video_before_motion", 0);
	m_video_after_motion = reader.GetInteger("motion_detection", "video_after_motion", 0);
	m_min_motion = reader.GetInteger("motion_detection", "minimum_motion", 0);
	m_cool_down = reader.GetInteger("motion_detection", "cool_down", 0);
	m_script_launcher = reader.Get("on_motion", "script_launcher", "UNKNOWN");
	m_script = reader.Get("on_motion", "script", "UNKNOWN");
	m_script_dir = reader.Get("on_motion", "script_dir", "UNKNOWN");
	m_output_dir = reader.Get("on_motion", "output_dir", "UNKNOWN");
	m_enable_display = reader.Get("display", "enable", "no");
	m_fps = static_cast<double>(reader.GetInteger("fps", "limit", 0));
	return true;
}

void MoDetSettings::__print()const {
	std::string str = "\ndevice\t\t\t:\t" + m_device + "\n";
	str += ("name\t\t\t:\t" + m_name + "\n");
	str += ("video before motion\t:\t" + std::to_string(m_video_before_motion) + "\n");
	str += ("video after motion\t:\t" + std::to_string(m_video_after_motion) + "\n");
	str += ("min motion seconds\t:\t" + std::to_string(m_min_motion) + "\n");
	str += ("cool down seconds\t:\t" + std::to_string(m_cool_down) + "\n");
	str += ("script launcher\t\t:\t" + m_script_launcher + "\n");
	str += ("script\t\t\t:\t" + m_script + "\n");
	str += ("script dir\t\t:\t" + m_script_dir + "\n");
	str += ("motion video dir\t:\t" + m_output_dir + "\n");
	str += ("enable display\t\t:\t" + m_enable_display + "\n");
	str += ("fps\t\t\t:\t" + std::to_string(m_fps) + "\n");
	AceLogger::Log(str);
}