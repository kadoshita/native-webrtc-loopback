cmake_minimum_required(VERSION 3.16)

include_directories(/Users/sublimer/workspace/webrtc/webrtc/include)
include_directories(/Users/sublimer/workspace/webrtc/webrtc/include/third_party/abseil-cpp)
link_directories(/Users/sublimer/workspace/webrtc/webrtc/lib)
add_executable(native-webrtc-loopback.out main.cpp)
target_compile_features(native-webrtc-loopback.out PUBLIC cxx_std_14)

target_link_libraries(native-webrtc-loopback.out
    "-framework AVFoundation"
    "-framework AudioUnit"
    "-framework AudioToolbox"
    "-framework CoreAudio"
    "-framework CoreGraphics"
    "-framework CoreMedia"
    "-framework CoreVideo"
    "-framework VideoToolbox"
    "-framework AppKit"
    "-framework OpenGL"
)

target_compile_definitions(native-webrtc-loopback.out PUBLIC
    WEBRTC_POSIX
    WEBRTC_MAC
)
target_link_libraries(native-webrtc-loopback.out webrtc)