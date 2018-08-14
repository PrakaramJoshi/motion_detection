#include "stdafx.h"
#include "IPCameraInFFmpeg.h"
#include "modet_settings.h"
#include "logger.h"
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <sstream>

using namespace AceLogger;
using namespace cv;

std::chrono::time_point<std::chrono::high_resolution_clock> GetTickCount() {
	return std::chrono::high_resolution_clock::now();
}
std::chrono::time_point<std::chrono::high_resolution_clock> TimePrev;
std::atomic<bool> open_stream;
//callback
static int cb_open(void *ctx)
{
	if (!open_stream) {
		return 0;
	}
	using namespace std::chrono_literals;
	//timeout after 3 seconds
	if (GetTickCount() - TimePrev > 60s)
	{
		AceLogger::LogErr("---interrupt_cb: Exit\n");
		return 1;
	}
	return 0;
}

FFmpegRTSPCapture::FFmpegRTSPCapture() {
	m_rtsp_path = "";

	img_convert_ctx = nullptr;
	format_ctx = nullptr;
	codec_ctx = nullptr;
	video_stream_index = 0;
	codec = nullptr;
	size2 = 0;
	m_fps = 0.0;
	picture = nullptr;
	picture_rgb = nullptr;
	picture_buffer_2 = nullptr;
}

FFmpegRTSPCapture::~FFmpegRTSPCapture() {
	clear();
}

void FFmpegRTSPCapture::init_ffmpegcapture() {
	// Register everything
	av_register_all();
	//av_log_set_level(AV_LOG_ERROR);
	/*avdevice_register_all();
	avcodec_register_all();
	avformat_network_init();*/
}

bool FFmpegRTSPCapture::init_rtspcapture() {
	AceLogger::Log("initializing rtsp...");
	open_stream = true;
	format_ctx = avformat_alloc_context();
	format_ctx->interrupt_callback.callback = cb_open;
	format_ctx->interrupt_callback.opaque = 0;

	codec_ctx = NULL;
	codec = NULL;
	//open RTSP
	//format_ctx->
	TimePrev = GetTickCount();
	AceLogger::Log("openning ");
	if (avformat_open_input(&format_ctx, m_rtsp_path.c_str(),
		NULL, NULL) != 0) {
		return false;
	}
	open_stream = false;
	AceLogger::Log("Finding stream info");
	if (avformat_find_stream_info(format_ctx, NULL) < 0) {
		return false;
	}

	//search video stream
	for (unsigned int i = 0; i < format_ctx->nb_streams; i++) {
		if (format_ctx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO)
			video_stream_index = i;
	}
	AceLogger::Log("intializing packet");
	av_init_packet(&packet);

	AceLogger::Log("Finding decoder");
	// Get the codec
	codec = avcodec_find_decoder(AV_CODEC_ID_H264);
	if (!codec) {
		return false;
	}
	AceLogger::Log("Allocating Context");
	// Add this to allocate the context by codec
	codec_ctx = avcodec_alloc_context3(codec);

	avcodec_get_context_defaults3(codec_ctx, codec);
	avcodec_copy_context(codec_ctx, format_ctx->streams[video_stream_index]->codec);

	AceLogger::Log("Checking context");
	if (avcodec_open2(codec_ctx, codec, NULL) < 0) {
		return false;
	}

	img_convert_ctx = sws_getContext(codec_ctx->width, codec_ctx->height,
		codec_ctx->pix_fmt, codec_ctx->width, codec_ctx->height, AV_PIX_FMT_BGR24,
		SWS_BICUBIC, NULL, NULL, NULL);

	AceLogger::Log("getting picture size");
	/*size2 = avpicture_get_size(AV_PIX_FMT_BGR24, codec_ctx->width,
		codec_ctx->height);*/

	size2 = av_image_get_buffer_size(AV_PIX_FMT_BGR24, codec_ctx->width ,codec_ctx->height, 1);

	AceLogger::Log("Allocating frames");
	picture = av_frame_alloc();
	picture_rgb = av_frame_alloc();

	picture_buffer_2 = (uint8_t*)(av_malloc(size2));
	/*avpicture_fill((AVPicture *)picture_rgb, picture_buffer_2, AV_PIX_FMT_BGR24,
		codec_ctx->width, codec_ctx->height);*/
	av_image_fill_arrays(picture_rgb->data, picture_rgb->linesize, picture_buffer_2, AV_PIX_FMT_BGR24, codec_ctx->width, codec_ctx->height, 1);
	//return picture;
	AceLogger::Log("intialized");
	return true;
}

double FFmpegRTSPCapture::get_fps_from_ffmpeg() {
	return av_q2d(format_ctx->streams[video_stream_index]->r_frame_rate);
}

int FFmpegRTSPCapture::get_width() {
	return codec_ctx->width;
}

int FFmpegRTSPCapture::get_height() {
	return codec_ctx->height;
}

bool FFmpegRTSPCapture::init_capture() {
	m_rtsp_path = MoDetSettings::get_device();
	init_ffmpegcapture();
	if (!init_rtspcapture()) {
		return false;
	}
	AceLogger::Log("Calculating fps");
	std::vector<double> fpss;
	for (int i = 0; i < 5; i++) {
		auto fps = determine_fps();
		AceLogger::Log("Fps for test " + std::to_string(i + 1) + " : " + std::to_string(fps));
		if (fps > 0) {
			fpss.push_back(fps);
		}
	}
	std::sort(fpss.begin(), fpss.end());

	double average_fps = 0.0;
	if (fpss.size() > 0) {
		auto sum = 0.0;
		for (auto i = 1; i < fpss.size() - 1; i++) {
			sum += fpss[i];
		}
		average_fps = sum / static_cast<double>(fpss.size()-2);
	}
	else {
		average_fps = get_fps_from_ffmpeg();
	}
	m_fps = average_fps;
	AceLogger::Log("calculated fps " + std::to_string(m_fps));
	if (m_fps > 1000.0 || m_fps <= 0.0) {
		return false;
	}
	m_fps_limiter.set_target_fps(MoDetSettings::get_fps());
	m_fps_limiter.set_camera_fps(m_fps);
	m_fps = m_fps_limiter.init();
	return true;
}

double FFmpegRTSPCapture::get_fps() const{
	return m_fps;
}


int decode(AVCodecContext *avctx, AVFrame *frame, int *got_frame, AVPacket *pkt)
{
	int ret;

	*got_frame = 0;

	if (pkt) {
		ret = avcodec_send_packet(avctx, pkt);
		// In particular, we don't expect AVERROR(EAGAIN), because we read all
		// decoded frames with avcodec_receive_frame() until done.
		if (ret < 0)
			return ret == AVERROR_EOF ? 0 : ret;
	}

	ret = avcodec_receive_frame(avctx, frame);
	if (ret < 0 && ret != AVERROR(EAGAIN) && ret != AVERROR_EOF)
		return ret;
	if (ret >= 0)
		*got_frame = 1;

	return 0;
}

double FFmpegRTSPCapture::determine_fps() {
	std::size_t max_retry_count = 10;
	double frame_count = 0;
	auto start = std::chrono::steady_clock::now();
	auto stop_time = std::chrono::steady_clock::now();
	auto delay = static_cast<double>(std::chrono::duration_cast<std::chrono::seconds>(stop_time - start).count());
	while (frame_count<10||delay<3) {
		auto ret = av_read_frame(format_ctx, &packet);
		if (ret == AVERROR(EAGAIN))
			continue;
		if (ret < 0)
		{
			if (ret == (int)AVERROR_EOF) {
				AceLogger::LogErr("End of file");
				clear();
				break;
			}
			else {
				AceLogger::LogErr("fail to av_read_frame");
				max_retry_count--;
				if (max_retry_count>0)
					continue;
				else {
					AceLogger::LogErr("Max retry timeout.");
					clear();
					break;
				}
			}
		}

		if (packet.stream_index == video_stream_index) {    //packet is video
			int check = 0;
			decode(codec_ctx, picture, &check, &packet);
			sws_scale(img_convert_ctx, picture->data, picture->linesize, 0,
				codec_ctx->height, picture_rgb->data, picture_rgb->linesize);
			cv::Mat *image = new Mat();
			cv::Mat(codec_ctx->height, codec_ctx->width, CV_8UC3, picture_rgb->data[0], picture_rgb->linesize[0]).copyTo(*image);
			delete image;
			frame_count++;
		}
		av_packet_unref(&packet);
		av_init_packet(&packet);
		stop_time = std::chrono::steady_clock::now();
		delay = static_cast<double>(std::chrono::duration_cast<std::chrono::seconds>(stop_time - start).count());
	}
	if (frame_count == 0.0) {
		return -1.0;
	}
	stop_time = std::chrono::steady_clock::now();
	delay = static_cast<double>(std::chrono::duration_cast<std::chrono::seconds>(stop_time - start).count());
	if (delay == 0.0) {
		return -1.0;
	}
	return frame_count / delay;
}

void FFmpegRTSPCapture::capture(BlockingQueue<cv::Mat>*_inq, std::atomic<bool> *_keep_alive) {
	std::size_t max_retry_count = 10;
	while (true) {
		auto ret = av_read_frame(format_ctx, &packet);
		if (ret == AVERROR(EAGAIN))
			continue;
		if (ret < 0)
		{
			if (ret == (int)AVERROR_EOF) {
				AceLogger::LogErr("End of file");
				clear();
				break;
			}
			else {
				AceLogger::LogErr("fail to av_read_frame");
				max_retry_count--;
				if (max_retry_count>0)
					continue;
				else {
					AceLogger::LogErr("Max retry timeout.");
					clear();
					break;
				}
			}
		}

		if (packet.stream_index == video_stream_index) {    //packet is video
			int check = 0;
			//int result = avcodec_decode_video2(codec_ctx, picture, &check, &packet);
			int result = decode(codec_ctx, picture, &check, &packet);
			

			if (m_fps_limiter.use_frame()) {
				
				if (result < 0) {
					AceLogger::LogErr("bad decode" + std::to_string(result));
				}
				if (check != 1) {
					AceLogger::Log("check result : " + std::to_string(check));
				}
				sws_scale(img_convert_ctx, picture->data, picture->linesize, 0,
					codec_ctx->height, picture_rgb->data, picture_rgb->linesize);
				cv::Mat *image = new Mat();
				cv::Mat(codec_ctx->height, codec_ctx->width, CV_8UC3, picture_rgb->data[0], picture_rgb->linesize[0]).copyTo(*image);
				_inq->Insert(image);
			}			
		}
		av_packet_unref(&packet);
		av_init_packet(&packet);
		if (!(*_keep_alive))
			break;
	}
}

void FFmpegRTSPCapture::clear() {
	av_packet_unref(&packet);
	if(picture)
		av_free(picture);
	picture = nullptr;
	if(picture_rgb)
		av_free(picture_rgb);
	picture_rgb = nullptr;
	if (picture_buffer_2) {
		av_free(picture_buffer_2);
	}
	picture_buffer_2 = nullptr;
	if (format_ctx) {
		avformat_close_input(&format_ctx);
		avformat_free_context(format_ctx);
	}
	format_ctx = nullptr;
}