#include "stream_puller.hh"
#include <boost/asio.hpp>
#include <iostream>
#include <spdlog/spdlog.h>
#include <nlohmann/json.hpp>
#include "utils/base64.hh"
#include "utils/socket_helper.hh"
extern "C" {
#include <libavformat/avformat.h>
#include <libavutil/time.h>
#include <libavcodec/avcodec.h>
}

using JSON = nlohmann::json;
using boost::asio::ip::tcp;

StreamPuller::StreamPuller(const std::string& stream_id, const std::string& ip, int port)
: stream_id_(stream_id), ip_(ip), port_(port) {}


StreamPuller::~StreamPuller() {
}

int StreamPuller::CodecFrameFromServer() {
    try {
        boost::asio::io_context io_context;
        tcp::socket socket(io_context);
        spdlog::info("Connecting to TCP server...");
        socket.connect(
            tcp::endpoint(boost::asio::ip::address::from_string(ip_),
                          port_));
        JSON start_pull;
        start_pull["type"] = MessageType::kTypeStreamInfo;
        start_pull["is_push"] = false;
        start_pull["stream_id"] = stream_id_;
        start_pull["enable"] = true;
        send_json(socket, start_pull);
        
        JSON codec_info;
        receive_json(socket, codec_info);
        
        if (codec_info["type"] != MessageType::kTypeCodecInfo) {
          spdlog::error("does not receive codec info");
          return 1;
        }
        
        AVCodecID codec_id = codec_info["codec_id"];
        
        int width = codec_info["width"];
        int height = codec_info["height"];
        int pix_fmt = codec_info["pix_fmt"];
        
        const AVCodec *codec = avcodec_find_decoder(codec_id);
        if (!codec) {
          spdlog::error("Codec not found");
          return 1;
        }

        AVCodecContext *codec_ctx = avcodec_alloc_context3(codec);
        
        codec_ctx->width = width;
        codec_ctx->height = height;
        codec_ctx->pix_fmt = (AVPixelFormat)pix_fmt;
        codec_ctx->extradata_size = codec_info["extradata_size"];
        
        auto data = base64_decode(codec_info["extradata"]);
        codec_ctx->extradata = data.data();
        
        if (!codec_ctx) {
          spdlog::error("Could not allocate video codec context");
          return 1;
        }
        
        if (avcodec_open2(codec_ctx, codec, NULL) < 0) {
          spdlog::error("Could not open codec");
          return 1;
        }
        
        auto frame = createAVFramePtr();

        while (true) {
          std::shared_ptr<AVPacket> paket = receive_packet(socket);
          int send_res = avcodec_send_packet(codec_ctx, paket.get());
          
          logAVPacket(paket.get());
          
          if (send_res == 0) {
            int ret = avcodec_receive_frame(codec_ctx, frame.get());

            if (ret == 0) {
              int width = frame->width;
              int height = frame->height;
              if (width <= 0 || height <= 0) {
                continue;
              }
              
//              v.OnVideoFrame(frame);
            }
          }
        }
      } catch (std::exception& e) {
        spdlog::error("Exception: {}", e.what());
        return 1;
      }
}

