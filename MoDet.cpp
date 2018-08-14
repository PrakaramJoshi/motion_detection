// MoDet.cpp : Defines the entry point for the console application.
//
#include "stdafx.h"
#include "logger.h"
#include "IPCameraInFFmpeg.h"
#include "DeviceCameraIn.h"
#include "motion_detector.h"
#include "video_display.h"
#include "CameraIn.h"
#include "modet_settings.h"
REGISTER_LOGGER("Iris", "0.0.0.1");
void process_frames(BlockingQueue<cv::Mat> *_framesq,const input_property  *_property) {
	motion_detector modet(_framesq, _property);
	modet.run();
}

void camera_run(CameraBase *_camera) {
	_camera->run();
}

int main(int argc,char *argv[])
{
	if (argc != 2) {
		std::cout << "usage : app_config_file" << std::endl;
		//Modet "rtsp://admin:Pegasus12@192.168.1.35/2" "c:\Python27\python.exe B:\Workspace\motion_detector\MoDet\MoDet\helium\home_automation\camera\email_modet.py" Backyard_Cam "B:\Workspace\motion_detector\MoDet\MoDet\helium\home_automation\camera" "B:\Workspace\motion_detector\MoDet\x64\ReleaseNoOpt\motion_video"
		return 1;
	}
	std::string app_config = argv[1];
	if (!MoDetSettings::read(app_config)) {
		return 1;
	}
	MoDetSettings::print();

	std::vector<BlockingQueue<cv::Mat> *>frames_qs;
	frames_qs.push_back(new BlockingQueue<cv::Mat>());
	display*display_object = nullptr;
	if (MoDetSettings::get_enable_display() == "yes") {
		frames_qs.push_back(new BlockingQueue<cv::Mat>());
		display_object=new display(frames_qs[1]);
	}

	input_property property;
	CameraBase *camera;
	if (MoDetSettings::get_device() == "webcam") {
		CameraIn<DeviceCapture>*cam = new CameraIn<DeviceCapture>(frames_qs,&property);
		camera = cam;
	}
	else {
		CameraIn<FFmpegRTSPCapture>*cam = new CameraIn<FFmpegRTSPCapture>(frames_qs, &property);
		camera = cam;
	}
	
	camera->init();
	std::thread frames_processor(std::bind(process_frames, frames_qs[0], &property));
	std::thread camera_processor(std::bind(camera_run, camera));
	if(display_object)
	display_object->run();
	camera_processor.join();
	frames_processor.join();
	for (auto q : frames_qs) {
		delete q;
	}
	delete camera;
	delete display_object;
    return 0;
}

