cmake_minimum_required(VERSION 3.10)
project(TimsMeetingClient)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_AUTOMOC ON)
cmake_policy(SET CMP0100 NEW)

find_package(Qt6 COMPONENTS Core Network Widgets REQUIRED)

find_package(spdlog REQUIRED)

add_executable(TimsMeetingClient
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
)
target_link_libraries(TimsMeetingClient
    Qt6::Core
    Qt6::Network
    Qt6::Widgets
    spdlog::spdlog
)
