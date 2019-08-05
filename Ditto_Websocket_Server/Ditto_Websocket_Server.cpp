#include "Websocket.h"
#include "Datarefs.h"
#include "XPLMDataAccess.h"
#include "XPLMProcessing.h"

broadcast_server server_instance{};
dataref new_data;
websocketpp::lib::thread asio_thread;

static float	listenCallback(
	float                inElapsedSinceLastCall,
	float                inElapsedTimeSinceLastFlightLoop,
	int                  inCounter,
	void* inRefcon);

PLUGIN_API int XPluginStart(
	char* outName,
	char* outSig,
	char* outDesc)
{
	strcpy_s(outName, 23, "Ditto Websocket Server");
	strcpy_s(outSig, 31, "phuong.x-plane.ditto.websocket");
	strcpy_s(outDesc, 28, "Ditto with Websocket Server");

	//Register to get callback every frame
	XPLMRegisterFlightLoopCallback(listenCallback, -1.0, nullptr);

	return 1;
}

PLUGIN_API void	XPluginStop(void)
{
	XPLMDebugString("Stopping Ditto.\n");
}

PLUGIN_API void XPluginDisable(void) {
	new_data.empty_list();
	server_instance.stop();
	// Wait for io_service to cleanly exit
	asio_thread.join();
	XPLMUnregisterFlightLoopCallback(listenCallback, nullptr);
	XPLMDebugString("Disabling Ditto.\n");
}

PLUGIN_API int  XPluginEnable(void) {
	if (!new_data.get_status()) {
		try {
			asio_thread = websocketpp::lib::thread(&broadcast_server::run, &server_instance);
			new_data.init();
		}
		catch (websocketpp::exception const& e) {
			XPLMDebugString(e.what());
			return 0;
		}
		XPLMRegisterFlightLoopCallback(listenCallback, -1.0, nullptr);
	}
	XPLMDebugString("Enabling Ditto.\n");
	return 1;
}

PLUGIN_API void XPluginReceiveMessage(XPLMPluginID inFrom, int inMsg, void* inParam) { }

float listenCallback(float inElapsedSinceLastCall,
	float inElapsedTimeSinceLastFlightLoop,
	int inCounter,
	void* inRefcon)
{
	const auto out_data = new_data.get_serialized_data();
	const auto size = new_data.get_serialized_size();

	auto verifier = flatbuffers::Verifier(out_data, size);
	if (Ditto::VerifyDataBuffer(verifier)) {
		server_instance.send(out_data, size);
	}
	else {
		XPLMDebugString("Flatbuffers verifier failed.\n");
	}

	new_data.reset_builder();
	return -1.0;
}