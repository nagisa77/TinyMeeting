cmake_minimum_required(VERSION 3.10)
project(TinyMeeting)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_AUTOMOC ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(MACOSX_BUNDLE_ICON_FILE icon.icns)
set(APP_ICON_MACOSX ${CMAKE_CURRENT_SOURCE_DIR}/icon.icns)
set_source_files_properties(${APP_ICON_MACOSX} PROPERTIES
    MACOSX_PACKAGE_LOCATION "Resources")

cmake_policy(SET CMP0100 NEW)

find_package(Qt6 COMPONENTS Core Network Widgets REQUIRED)

find_package(spdlog REQUIRED)
find_package(nlohmann_json REQUIRED)
find_package(Boost REQUIRED COMPONENTS system thread)
find_library(COCOA_LIBRARY Cocoa)

find_library(SCREEN_CAPTURE_KIT_LIBRARY ScreenCaptureKit)

find_library(AVFOUNDATION_LIBRARY AVFoundation)

find_path(FFMPEG_INCLUDE_DIR libavformat/avformat.h)
find_library(AVFORMAT_LIBRARY avformat)
find_library(AVCODEC_LIBRARY avcodec)
find_library(AVUTIL_LIBRARY avutil)
find_library(SWSCALE_LIBRARY swscale)
find_library(SWRESAMPLE_LIBRARY swresample)

find_library(COREMEDIA_LIBRARY CoreMedia)
find_library(COREVIDEO_LIBRARY CoreVideo)

set(CAMERA_USAGE_DESCRIPTION "此应用需要使用摄像头来进行会议视频通话")

qt_add_resources(RESOURCES resources.qrc) # 添加资源文件

add_executable(TinyMeeting
    MACOSX_BUNDLE ${APP_ICON_MACOSX}

    main.cc
    pre_meeting_view.cc
    pre_meeting_view.hh
    pre_meeting_controller.cc
    pre_meeting_controller.hh
    meeting_model.cc
    meeting_model.hh
    http_protocol.cc
    http_protocol.hh
    http_service.cc
    http_service.hh
    in_meeting_controller.cc
    in_meeting_controller.hh
    in_meeting_view.cc
    in_meeting_view.hh
    window_manager.cc
    window_manager.hh
    stream_pusher.hh
    stream_pusher.cc
    stream_puller.hh
    stream_puller.cc

    header.hh

    media_capture/camera_capture.hh
    media_capture/camera_capture.mm
    media_capture/frame.hh
    media_capture/frame.mm

    utils/blocking_queue.hh
    utils/base64.hh
    utils/socket_helper.hh

    ui_utils/toast.hh

    ${RESOURCES}
)
set_target_properties(TinyMeeting PROPERTIES
    MACOSX_BUNDLE_INFO_PLIST ${CMAKE_CURRENT_SOURCE_DIR}/Info.plist
    XCODE_ATTRIBUTE_NS_CAMERA_USAGE_DESCRIPTION "${CAMERA_USAGE_DESCRIPTION}"
    )

target_link_libraries(TinyMeeting
    Qt6::Core
    Qt6::Network
    Qt6::Widgets
    spdlog::spdlog
    Boost::system
    Boost::thread
    nlohmann_json::nlohmann_json
    ${COCOA_LIBRARY}
    ${SCREEN_CAPTURE_KIT_LIBRARY}
    ${AVFOUNDATION_LIBRARY}
    ${AVFORMAT_LIBRARY}
    ${AVCODEC_LIBRARY}
    ${AVUTIL_LIBRARY}
    ${SWSCALE_LIBRARY}
    ${COREMEDIA_LIBRARY}
    ${COREVIDEO_LIBRARY}
)
