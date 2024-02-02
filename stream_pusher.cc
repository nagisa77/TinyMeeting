#include "stream_pusher.hh"
#include <boost/asio.hpp>
#include <iostream>
#include <spdlog/spdlog.h>
#include <nlohmann/json.hpp>
#include "utils/base64.hh"
extern "C" {
#include <libavformat/avformat.h>
#include <libavutil/time.h>
#include <libavcodec/avcodec.h>
}

AVFrame* AVFRAME2AVframe(std::shared_ptr<AVFRAME> frame) {
  return (AVFrame*)frame->frame_;
}

enum MessageType {
  kTypeStreamInfo = 0,
  kTypePacket = 1,
  kTypeCodecInfo = 2,
};

void logAVPacket(const AVPacket* pkt) {
    std::stringstream ss;
    
    ss << "AVPacket: ";
    ss << "pts = " << pkt->pts << ", ";
    ss << "dts = " << pkt->dts << ", ";
    ss << "size = " << pkt->size << ", ";
    ss << "stream_index = " << pkt->stream_index << ", ";
    ss << "flags = " << pkt->flags << ", ";
    ss << "side_data_elems = " << pkt->side_data_elems << ", ";
    ss << "duration = " << pkt->duration << ", ";
    ss << "pos = " << pkt->pos << ", ";

    // 打印 data 字段的前几个字节
    ss << "data (first few bytes) = ";
    const int dataBytesToPrint = 10;
    for (int i = 0; i < std::min(pkt->size, dataBytesToPrint); ++i) {
        ss << std::setfill('0') << std::setw(2) << std::hex << (int)pkt->data[i] << " ";
    }

    // 打印 side_data 字段的前几个字节（如果存在）
    if (pkt->side_data_elems > 0 && pkt->side_data != nullptr) {
        ss << ", side_data (first few bytes) = ";
        const int sideDataBytesToPrint = 10;
        for (int i = 0; i < std::min((int)pkt->side_data->size, sideDataBytesToPrint); ++i) {
            ss << std::setfill('0') << std::setw(2) << std::hex << (int)pkt->side_data->data[i] << " ";
        }
    }

    spdlog::info(ss.str());
}


struct AVFrameDeleter {
  void operator()(AVFrame* frame) const {
    if (frame) {
      av_frame_free(&frame);
    }
  }
};

using AVFramePtr = std::shared_ptr<AVFrame>;

inline AVFramePtr createAVFramePtr() {
    return AVFramePtr(av_frame_alloc(), AVFrameDeleter());
}

using JSON = nlohmann::json;

using boost::asio::ip::tcp;

static void send_json(tcp::socket& socket, const JSON& json) {
  std::string json_str = json.dump();
  spdlog::info("send json: {}", json_str);
  size_t json_length = json_str.size();
  std::vector<boost::asio::const_buffer> buffers;
  buffers.push_back(boost::asio::buffer(&json_length, sizeof(json_length)));
  buffers.push_back(boost::asio::buffer(json_str));
  boost::asio::write(socket, buffers);
}

static void receive_json(tcp::socket& socket, JSON& json) {
  std::uint32_t json_length;
  boost::asio::read(socket, boost::asio::buffer(&json_length, sizeof(json_length)));
  std::vector<char> json_str(json_length);
  boost::asio::read(socket, boost::asio::buffer(json_str.data(), json_length));
  json = JSON::parse(json_str.begin(), json_str.end());
}

static void send_packet(tcp::socket& socket, const AVPacket* pkt) {
  auto packet_size = std::make_shared<std::uint32_t>(pkt->size);
  
  JSON json;
//  json["packet_size"] = *packet_size + 4;
  json["packet_size"] = *packet_size;
  json["type"] = MessageType::kTypePacket;
  json["pts"] = pkt->pts;
  json["dts"] = pkt->dts;
  json["stream_index"] = pkt->stream_index;
  json["duration"] = pkt->duration;
  json["pos"] = pkt->pos;
  
  send_json(socket, json);
  
  logAVPacket(pkt);
  
  auto packet_data = std::make_shared<std::vector<char>>(*packet_size);

  // 复制原始数据
  std::copy(pkt->data, pkt->data + pkt->size, packet_data->begin());

  // 发送数据
  std::vector<boost::asio::const_buffer> buffers;
  buffers.push_back(boost::asio::buffer(*packet_data));

  boost::asio::write(socket, buffers);
}

StreamPusher::StreamPusher(const std::string& stream_id, const std::string& ip, int port)
  : frame_queue_(100), stream_id_(stream_id), ip_(ip), port_(port) {
    codec_thread_ = std::make_unique<boost::thread>(&StreamPusher::CodecFrameToServer, this);
}

StreamPusher::~StreamPusher() {
  // 在析构函数中停止线程
  if (codec_thread_ && codec_thread_->joinable()) {
//      codec_thread_->interrupt();  如果您的 CodecFrameToServer 方法支持中断，请使用这个
      codec_thread_->join();
  }
}

void StreamPusher::OnCameraFrame(std::shared_ptr<AVFRAME> frame) {
  frame_queue_.push(frame);
}

int StreamPusher::CodecFrameToServer() {
  spdlog::info("Starting Pusher client...");

    try {
      boost::asio::io_context io_context;
      tcp::socket socket(io_context);
      spdlog::info("Connecting to TCP server...");
      socket.connect(
          tcp::endpoint(boost::asio::ip::address::from_string(ip_),
                        port_)); // 替换为您的 TCP 端口
      JSON start_push;
      start_push["type"] = MessageType::kTypeStreamInfo;
      start_push["is_push"] = true;
      start_push["stream_id"] = stream_id_;
      start_push["enable"] = true;
      send_json(socket, start_push);
      
      spdlog::info("start Codec");
      
      const AVCodec* codec = nullptr;
      AVCodecContext* pCodecCtx = nullptr;
      
      while (auto AVF = frame_queue_.pop()) {
        auto frame = AVFRAME2AVframe(AVF);
        if (!codec || !pCodecCtx) {
          // 30fps
          AVRational stream_time_base;
          stream_time_base.num = 1;
          stream_time_base.den = 30; // 这里感觉有问题，需要弄成不是固定1s 30fps，不然卡卡的
          
          codec = avcodec_find_encoder(AVCodecID::AV_CODEC_ID_H264); // 以 H.264 为例
          pCodecCtx = avcodec_alloc_context3(codec);
          
          // 配置 codecContext，例如设置比特率、分辨率等参数
          pCodecCtx->width = frame->width;
          pCodecCtx->height = frame->height;
          pCodecCtx->pix_fmt = (AVPixelFormat)frame->format;
          pCodecCtx->time_base = stream_time_base;
          
          if (avcodec_open2(pCodecCtx, codec, NULL) < 0) {
            fprintf(stderr, "Could not open codec\n");
            return 1;
          }
        }
        
        static bool send_codec_info = false;
        
        if (!send_codec_info) {
          JSON codec_info;
          start_push["type"] = MessageType::kTypeCodecInfo;
          start_push["codec_id"] = pCodecCtx->codec_id;
          start_push["width"] = pCodecCtx->width;
          start_push["height"] = pCodecCtx->height;
          start_push["pix_fmt"] = pCodecCtx->pix_fmt;
          start_push["extradata_size"] = pCodecCtx->extradata_size;
          start_push["extradata"] = base64_encode(pCodecCtx->extradata, pCodecCtx->extradata_size);
          send_json(socket, start_push);
          
          send_codec_info = true;
        }
        
        AVPacket* pkt = av_packet_alloc();

        // 为每一个AVFrame编码生成AVPacket
        if (avcodec_send_frame(pCodecCtx, frame) == 0) {
          while (avcodec_receive_packet(pCodecCtx, pkt) == 0) {
            // 此时pkt包含编码后的视频帧数据，可以发送
            send_packet(socket, pkt);  // 假设这个函数已经实现，用于通过socket发送pkt

            av_packet_unref(pkt);  // 必须调用，以避免内存泄漏
          }
        }
        
        // 释放 AVPacket
        av_packet_free(&pkt);
      }
      
      // free
      avcodec_free_context(&pCodecCtx);
      
//        JSON stop_push;
//        stop_push["type"] = MessageType::kTypeStreamInfo;
//        stop_push["is_push"] = true;
//        stop_push["stream_id"] = "sub-video-tim";
//        stop_push["enable"] = false;
//        send_json(socket, stop_push);
    } catch (std::exception& e) {
      spdlog::error("Exception: {}", e.what());
      return 1;
    }

    spdlog::info("Client finished successfully.");
    return 0;
}

