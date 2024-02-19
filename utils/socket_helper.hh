#ifndef SOCKET_HELPER_HH
#define SOCKET_HELPER_HH

#include <boost/asio.hpp>
#include <iostream>
#include <spdlog/spdlog.h>
#include <nlohmann/json.hpp>
#include "base64.hh"
extern "C" {
#include <libavformat/avformat.h>
#include <libavutil/time.h>
#include <libavcodec/avcodec.h>
}

enum MessageType {
  kTypeStreamInfo = 0,
  kTypePacket = 1,
  kTypeCodecInfo = 2,
};

static void logAVPacket(const AVPacket* pkt) {
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

static std::shared_ptr<AVPacket> receive_packet(tcp::socket& socket) {
  JSON json;
  receive_json(socket, json);
  
  int type = json["type"];
  int size = json["packet_size"];
  
  char* packet_data = (char*) malloc(size * sizeof(char)); // leak
  boost::asio::read(socket, boost::asio::buffer(packet_data, size));
  static auto av_packet_deleter = [](AVPacket* pkt) {
    av_packet_free(&pkt);
  };
  std::shared_ptr<AVPacket> packet(av_packet_alloc(), av_packet_deleter);
  av_packet_from_data(packet.get(), (uint8_t*)packet_data, size);
  
  packet->pts = json["pts"];
  packet->dts = json["dts"];
  packet->stream_index = json["stream_index"];
  packet->duration = json["duration"];
  packet->pos = json["pos"];
  
  return packet;
}


#endif // SOCKET_HELPER_HH
