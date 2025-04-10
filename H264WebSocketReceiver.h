#pragma once

#include <ixwebsocket/IXWebSocket.h>
#include <gst/gst.h>
#include <gst/app/gstappsrc.h>
#include <vector>
#include <mutex>

#include "Info.h"

class H264WebSocketReceiver {
public:
    H264WebSocketReceiver(const std::string& url, GstElement* appsrc);

    void init();     // WebSocket 설정
    void auth();     // 인증 요청
    void receive();  // WebSocket 시작

    void stop();

private:
    void onMessage(const ix::WebSocketMessagePtr& msg);
    void processH264Stream(const std::vector<uint8_t>& data);
    void pushToGStreamer(const uint8_t* data, size_t size);

    std::vector<uint8_t> makePacket(uint8_t requestType);

    std::string m_url;
    GstElement* m_appsrc;
    ix::WebSocket m_websocket;
    std::mutex m_mutex;
    std::vector<uint8_t> m_buffer;
    bool m_isAuth = false;
};
