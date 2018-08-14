#pragma once
#pragma once
#include "BlockingQueue.h"
#include "input_property.h"
#include "opencv_include.h"
#include "ffmpeg_include.h"
#include "fps_limiter.h"
#include <atomic>

class FFmpegRTSPCapture {
	std::string m_rtsp_path;

	SwsContext *img_convert_ctx;
	AVFormatContext* format_ctx;
	AVCodecContext* codec_ctx;
	int video_stream_index;
	AVPacket packet;
	AVCodec *codec;
	int size2;
	AVFrame* picture;
	AVFrame* picture_rgb;
	uint8_t* picture_buffer_2;
	double m_fps;
	fps_limiter m_fps_limiter;

	void init_ffmpegcapture();

	bool init_rtspcapture();

	double determine_fps();

	double get_fps_from_ffmpeg();

public:

	FFmpegRTSPCapture();

	bool init_capture();

	void capture(BlockingQueue<cv::Mat>*_inq, std::atomic<bool> *_keep_alive);

	double get_fps()const;

	int get_width();

	int get_height();

	void clear();

	~FFmpegRTSPCapture();
};