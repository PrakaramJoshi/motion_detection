#pragma once
#include "input_property.h"
#include "opencv_include.h"
#include "ffmpeg_include.h"
#include <atomic>
#include "logger.h"
#ifndef CV_FOURCC
//#define CV_FOURCC(a,b,c,d) cv::VideoWriter::fourcc(a,b,c,d);
#endif
inline void video_write_opencv(const std::string &_outfile, std::vector<cv::Mat*> &_data, const input_property *_input_property) {
	auto fourcc = CV_FOURCC('M', 'J', 'P', 'G');
	auto writer = cv::VideoWriter(_outfile, fourcc, _input_property->get_fps(), _input_property->get_size(), true);
	std::for_each(_data.begin(), _data.end(),
		[&writer](cv::Mat *_f) {writer.write(*_f); });
}
//inline int video_writer(const std::string &_outfile,std::vector<cv::Mat*> &_data,const input_property *_input_property)
//{
//
//	// initialize FFmpeg library
//	av_register_all();
//	//  av_log_set_level(AV_LOG_DEBUG);
//	int ret;
//	const int dst_width = _input_property->get_size().width;
//	const int dst_height = _input_property->get_size().height;
//	const AVRational dst_fps = { 1, 1 };
//
//
//	// open output format context
//	AVFormatContext* outctx = nullptr;
//	ret = avformat_alloc_output_context2(&outctx, nullptr, nullptr, _outfile.c_str());
//	if (ret < 0) {
//		std::cerr << "fail to avformat_alloc_output_context2(" << _outfile << "): ret=" << ret;
//		return 2;
//	}
//
//	// open output IO context
//	ret = avio_open2(&outctx->pb, _outfile.c_str(), AVIO_FLAG_WRITE, nullptr, nullptr);
//	if (ret < 0) {
//		std::cerr << "fail to avio_open2: ret=" << ret;
//		return 2;
//	}
//
//	// create new video stream
//	AVCodec* vcodec = avcodec_find_encoder(outctx->oformat->video_codec);
//	AVStream* vstrm = avformat_new_stream(outctx, vcodec);
//	if (!vstrm) {
//		std::cerr << "fail to avformat_new_stream";
//		return 2;
//	}
//	avcodec_get_context_defaults3(vstrm->codecpar, vcodec);
//	vstrm->codec->width = dst_width;
//	vstrm->codec->height = dst_height;
//	vstrm->codec->pix_fmt = vcodec->pix_fmts[0];
//	vstrm->codec->time_base = vstrm->time_base = av_inv_q(dst_fps);
//	vstrm->r_frame_rate = vstrm->avg_frame_rate = dst_fps;
//	if (outctx->oformat->flags & AVFMT_GLOBALHEADER)
//		vstrm->codec->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
//
//	// open video encoder
//	ret = avcodec_open2(vstrm->codec, vcodec, nullptr);
//	if (ret < 0) {
//		std::cerr << "fail to avcodec_open2: ret=" << ret;
//		return 2;
//	}
//
//	// initialize sample scaler
//	SwsContext* swsctx = sws_getCachedContext(
//		nullptr, dst_width, dst_height, AV_PIX_FMT_BGR24,
//		dst_width, dst_height, vstrm->codec->pix_fmt, SWS_BICUBIC, nullptr, nullptr, nullptr);
//	if (!swsctx) {
//		std::cerr << "fail to sws_getCachedContext";
//		return 2;
//	}
//
//	// allocate frame buffer for encoding
//	AVFrame* frame = av_frame_alloc();
//	std::vector<uint8_t> framebuf(av_image_get_buffer_size(vstrm->codec->pix_fmt, dst_width, dst_height, 16));
//
//	av_image_fill_arrays(frame->data, frame->linesize, framebuf.data(), vstrm->codec->pix_fmt, dst_width, dst_height, 1);
//	//avpicture_fill(reinterpret_cast<AVPicture*>(frame), framebuf.data(), vstrm->codec->pix_fmt, dst_width, dst_height);
//	frame->width = dst_width;
//	frame->height = dst_height;
//	frame->format = static_cast<int>(vstrm->codec->pix_fmt);
//
//	// encoding loop
//	avformat_write_header(outctx, nullptr);
//	int64_t frame_pts = 0;
//	unsigned nb_frames = 0;
//	bool end_of_stream = false;
//	int got_pkt = 0;
//	auto iter = _data.begin();
//	end_of_stream = iter == _data.end();
//	do {
//		
//		if (!end_of_stream) {
//			// convert cv::Mat(OpenCV) to AVFrame(FFmpeg)
//			const int stride[] = { static_cast<int>((*iter)->step[0]) };
//			sws_scale(swsctx, &(*iter)->data, stride, 0, (*iter)->rows, frame->data, frame->linesize);
//			frame->pts = frame_pts++;
//		}
//		// encode video frame
//		AVPacket pkt;
//		pkt.data = nullptr;
//		pkt.size = 0;
//		av_init_packet(&pkt);
//		ret = avcodec_encode_video2(vstrm->codec, &pkt, end_of_stream ? nullptr : frame, &got_pkt);
//		if (ret < 0) {
//			AceLogger::LogErr("fail to avcodec_encode_video2: ret=" + std::to_string(ret));
//			break;
//		}
//		if (got_pkt) {
//			// rescale packet timestamp
//			pkt.duration = 1;
//			av_packet_rescale_ts(&pkt, vstrm->codec->time_base, vstrm->time_base);
//			// write packet
//			av_write_frame(outctx, &pkt);
//			AceLogger::Log("Writing video data " + std::to_string(nb_frames));
//			++nb_frames;
//		}
//		av_packet_unref(&pkt);
//		++iter;
//		end_of_stream = end_of_stream || (iter == _data.end());
//	} while (!end_of_stream || got_pkt);
//	av_write_trailer(outctx);
//	
//
//	av_frame_free(&frame);
//	avcodec_close(vstrm->codec);
//	avio_close(outctx->pb);
//	avformat_free_context(outctx);
//	return 0;
//}
