#pragma once
#include <websocketpp/config/asio_no_tls.hpp>
#include <websocketpp/server.hpp>
#include <websocketpp/common/thread.hpp>
#include <set>

typedef websocketpp::server<websocketpp::config::asio> server;

class broadcast_server {
public:
	broadcast_server();
	void run();
	void send(uint8_t* send_buf, int size);
	void on_open(websocketpp::connection_hdl hdl);
	void on_close(websocketpp::connection_hdl hdl);
	void on_message(websocketpp::connection_hdl hdl, server::message_ptr msg);

private:
	server m_server;
	std::set<websocketpp::connection_hdl, std::owner_less<websocketpp::connection_hdl>> m_connections;
	websocketpp::lib::mutex m_action_lock;
	websocketpp::lib::mutex m_connection_lock;
};