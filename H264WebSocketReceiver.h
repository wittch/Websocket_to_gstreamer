#pragma once
//#include <websocketpp/config/asio_client.hpp>
//#include <websocketpp/client.hpp>
//#include <websocketpp/common/memory.hpp>
//#include <websocketpp/common/thread.hpp>
//#include <websocketpp/server.hpp>
//#include <websocketpp/config/asio.hpp>
//#include <websocketpp/config/asio_no_tls.hpp>

#include <ixwebsocket/IXNetSystem.h>
#include <ixwebsocket/IXSocketTLSOptions.h>
#include <ixwebsocket/IXWebSocket.h>
#include <ixwebsocket/IXWebSocketServer.h>
#include <ixwebsocket/IXWebSocketErrorInfo.h>


#include <openssl/sha.h>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <thread>
#include <mutex>
#include <vector>
#include <gst/gst.h>
#include <gst/app/gstappsrc.h>

//using namespace websocketpp;
//typedef server<websocketpp::config::asio_tls> wss;
//typedef server<websocketpp::config::asio> ws;
//
//typedef websocketpp::client<websocketpp::config::asio_tls_client> ws_client;

class H264WebSocketReceiver {
public:
    H264WebSocketReceiver(const std::string& url, GstElement* appsrc);
	void init();
	void auth();
	void receive();
	void stop();
	std::vector<uint8_t> makePacket(uint8_t requestType);
	void processH264Stream(const std::vector<uint8_t>& data);
	void pushToGStreamer(const uint8_t* data, size_t size);
	void onMessage(const ix::WebSocketMessagePtr& msg);
private:
	std::string m_url;
	GstElement* m_appsrc;
	ix::WebSocket m_websocket;
	std::mutex m_mutex;
	std::vector<uint8_t> m_buffer;
	bool m_isAuth = false;
};
