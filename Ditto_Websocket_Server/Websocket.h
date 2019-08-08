#pragma once
#include <websocketpp/config/asio_no_tls.hpp>
#include <websocketpp/server.hpp>
#include <websocketpp/common/thread.hpp>
#include <optional>
#include <set>
#include "Utility.h"

typedef websocketpp::server<websocketpp::config::asio> server;

class broadcast_server {
public:
	broadcast_server();
	void run();
	void send(uint8_t* send_buf, size_t size);
	void stop();
	int get_port_number();
private:
	std::string plugin_path_{};
	int port_number_{};
	server m_server;
	std::set<websocketpp::connection_hdl, std::owner_less<websocketpp::connection_hdl>> m_connections;
	websocketpp::lib::mutex m_connection_lock;
	void on_open(websocketpp::connection_hdl hdl);
	void on_close(websocketpp::connection_hdl hdl);
	void on_message(websocketpp::connection_hdl hdl, server::message_ptr msg);
	void set_plugin_path(std::string path);
};