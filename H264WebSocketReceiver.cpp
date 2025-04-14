#include "H264WebSocketReceiver.h"




H264WebSocketReceiver::H264WebSocketReceiver(const std::string& url, GstElement* appsrc)
    : m_url(url), m_appsrc(appsrc) {
}

void H264WebSocketReceiver::init() {

    // Windows에서 필수!
    ix::initNetSystem();
    ix::SocketTLSOptions tlsOptions;
    tlsOptions.tls = true;
    //tlsOptions.certFile = "C:\\inetpub\\wwwroot\\cert.pem"; // 인증서 파일 경로
    //tlsOptions.keyFile = "C:\\inetpub\\wwwroot\\key.pem";   // 개인 키 파일 경로
    tlsOptions.caFile = "NONE";  // 인증서 검증 끔
    //tlsOptions.caFile = "C:\\inetpub\\wwwroot\\cert.pem";
    tlsOptions.disable_hostname_validation = true; // CN 검사도 끔
    m_websocket.setTLSOptions(tlsOptions);

    m_websocket.setUrl(m_url);

    m_websocket.setOnMessageCallback([this](const ix::WebSocketMessagePtr& msg) {
        onMessage(msg);
        });
}

void H264WebSocketReceiver::auth() {
	
    //auto packet = makePacket(REQ_Auth);
    //m_websocket.sendBinary(std::string(packet.begin(), packet.end()));
}

void H264WebSocketReceiver::receive() {
    m_websocket.start();
}

void H264WebSocketReceiver::stop() {
    m_websocket.stop();
}

void H264WebSocketReceiver::onMessage(const ix::WebSocketMessagePtr& msg) {
    if (msg->type == ix::WebSocketMessageType::Open) {
        std::cout << "WebSocket connected" << std::endl;
        //auth();
    }
    else if (msg->type == ix::WebSocketMessageType::Message) {
        const std::string& payload = msg->str;

        /*if (!m_isAuth) {
            const uint8_t* data = reinterpret_cast<const uint8_t*>(payload.data());
            if (payload.size() >= 14 && data[0] == 0x68 && data[1] == 0x76 &&
                data[2] == 0x11 && data[4] == 0x02 && data[5] == 0x01 && data[12] == 0x00) {
                std::cout << "Auth success" << std::endl;
                m_isAuth = true;
                auto startPacket = makePacket(REQ_StartAll);
                m_websocket.sendBinary(std::string(startPacket.begin(), startPacket.end()));
            }
            else {
                std::cerr << "Auth failed, retrying in 5 seconds..." << std::endl;
                std::this_thread::sleep_for(std::chrono::seconds(5));
                auth();
            }
            return;
        }*/

        std::vector<uint8_t> data(payload.begin(), payload.end());
        processH264Stream(data);
    }
    else if (msg->type == ix::WebSocketMessageType::Close) {
        std::cout << "WebSocket closed" << std::endl;
        m_isAuth = false;
    }
    else if (msg->type == ix::WebSocketMessageType::Error) {
        std::cerr << "WebSocket error: " << msg->errorInfo.reason << std::endl;

    }
}

std::vector<uint8_t> H264WebSocketReceiver::makePacket(uint8_t requestType) {
    std::vector<uint8_t> packet;
    packet.push_back(0x68); packet.push_back(0x76); // magic
    packet.push_back(0x11); packet.push_back(0x00); // protocol_type
    packet.push_back(0x02); packet.push_back(0x01); packet.push_back(0x00); packet.push_back(0x00); // protocol_mode
    uint32_t bodySize = 1;
    packet.push_back((bodySize >> 0) & 0xFF);
    packet.push_back((bodySize >> 8) & 0xFF);
    packet.push_back((bodySize >> 16) & 0xFF);
    packet.push_back((bodySize >> 24) & 0xFF);
    packet.push_back(requestType); // body
    return packet;
}

void H264WebSocketReceiver::processH264Stream(const std::vector<uint8_t>& data) {
    std::lock_guard<std::mutex> lock(m_mutex);
    //m_buffer.insert(m_buffer.end(), data.begin(), data.end());

    //if (m_buffer.size() > 1024 * 1024 * 4) { // 4MB 제한
    //    std::cerr << "Buffer overflow, clearing..." << std::endl;
    //    m_buffer.clear();
    //    return;
    //}


    pushToGStreamer(data.data(), data.size());

    //size_t pos = 0;
    /*while (pos + 4 < m_buffer.size()) {
        size_t start = pos;
        if (m_buffer[pos] == 0x00 && m_buffer[pos + 1] == 0x00 &&
            ((m_buffer[pos + 2] == 0x01) || (m_buffer[pos + 2] == 0x00 && m_buffer[pos + 3] == 0x01))) {
            size_t next = pos + 4;
            while (next + 4 < m_buffer.size() &&
                !(m_buffer[next] == 0x00 && m_buffer[next + 1] == 0x00 &&
                    (m_buffer[next + 2] == 0x01 || (m_buffer[next + 2] == 0x00 && m_buffer[next + 3] == 0x01)))) {
                ++next;
            }

            size_t nal_size = next - pos;
            pushToGStreamer(m_buffer.data() + pos, nal_size);
            pos = next;
        }
        else {
            ++pos;
        }
    }*/

    //m_buffer.erase(m_buffer.begin(), m_buffer.begin() + pos);
}

void H264WebSocketReceiver::pushToGStreamer(const uint8_t* data, size_t size) {
    if (!m_appsrc) return;

    GstBuffer* buffer = gst_buffer_new_allocate(nullptr, size, nullptr);
    gst_buffer_fill(buffer, 0, data, size);
    g_print("push buffer\n");
    GstFlowReturn ret;
    g_signal_emit_by_name(m_appsrc, "push-buffer", buffer, &ret);
    gst_buffer_unref(buffer);

    if (ret != GST_FLOW_OK) {
        std::cerr << "Failed to push buffer to appsrc" << std::endl;
    }
}
