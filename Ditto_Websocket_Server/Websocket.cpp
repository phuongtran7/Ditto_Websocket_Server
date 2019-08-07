#include "Websocket.h"

broadcast_server::broadcast_server() {
	// Initialize Asio Transport
	m_server.init_asio();

	// Register handler callbacks
	m_server.set_open_handler(websocketpp::lib::bind(&broadcast_server::on_open, this, websocketpp::lib::placeholders::_1));
	m_server.set_close_handler(websocketpp::lib::bind(&broadcast_server::on_close, this, websocketpp::lib::placeholders::_1));
	m_server.set_message_handler(websocketpp::lib::bind(&broadcast_server::on_message, this, websocketpp::lib::placeholders::_1, websocketpp::lib::placeholders::_2));
}

void broadcast_server::stop() {
	m_server.stop_listening();
	websocketpp::lib::lock_guard<websocketpp::lib::mutex> guard(m_connection_lock);
	if (!m_connections.empty())
	{
		// Closing all outstanding connections with normal (1000) code
		for (auto it = m_connections.begin(); it != m_connections.end(); ++it) {
			m_server.close(*it, websocketpp::close::status::normal, "");
		}
	}
}

void broadcast_server::run() {

	if (m_server.stopped()) {
		m_server.reset();
	}

	// listen on specified port
	m_server.listen(1234);

	// Start the server accept loop
	m_server.start_accept();

	// Start the ASIO io_service run loop
	try {
		m_server.run();
	}
	catch (const websocketpp::exception& e) {
		std::cout << e.what() << std::endl;
	}
}

void broadcast_server::send(uint8_t* send_buf, size_t size)
{
	websocketpp::lib::lock_guard<websocketpp::lib::mutex> guard(m_connection_lock);
	if (!m_connections.empty())
	{
		for (auto it = m_connections.begin(); it != m_connections.end(); ++it) {
			// No need to convert to string. Send binary buffer directly
			// more info: https://github.com/zaphoyd/websocketpp/issues/572#issuecomment-235865551
			m_server.send(*it, send_buf, size, websocketpp::frame::opcode::binary);
		}
	}
}

void broadcast_server::on_open(websocketpp::connection_hdl hdl) {
	websocketpp::lib::lock_guard<websocketpp::lib::mutex> guard(m_connection_lock);
	m_connections.insert(hdl);
}

void broadcast_server::on_close(websocketpp::connection_hdl hdl) {
	websocketpp::lib::lock_guard<websocketpp::lib::mutex> guard(m_connection_lock);
	m_connections.erase(hdl);
}

void broadcast_server::on_message(websocketpp::connection_hdl hdl, server::message_ptr msg) {
}

void broadcast_server::set_plugin_path(std::string path)
{
	plugin_path_ = path;
}
