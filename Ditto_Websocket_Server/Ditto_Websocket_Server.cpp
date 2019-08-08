#include "Websocket.h"
#include "Datarefs.h"

broadcast_server server_instance{};
dataref new_data;
websocketpp::lib::thread asio_thread;
XPLMFlightLoopID data_flight_loop_id{};
XPLMFlightLoopID retry_flight_loop_id{};

static float	data_callback(
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
	if (data_flight_loop_id != nullptr) {
		XPLMDestroyFlightLoop(data_flight_loop_id);
	}
	XPLMDebugString("Disabling Ditto.\n");
}

PLUGIN_API int  XPluginEnable(void) {
	if (!new_data.get_status()) {
		try {
			asio_thread = websocketpp::lib::thread(&broadcast_server::run, &server_instance);
			new_data.init();

			if (new_data.get_not_found_list_size() != 0) {
				// Register a new flight loop to retry finding datarefs
				XPLMCreateFlightLoop_t params = { sizeof(XPLMCreateFlightLoop_t), xplm_FlightLoop_Phase_AfterFlightModel,  retry_callback, nullptr };
				data_flight_loop_id = XPLMCreateFlightLoop(&params);
				if (data_flight_loop_id != nullptr)
				{
					XPLMScheduleFlightLoop(retry_flight_loop_id, -1, true);
				}
			}
		}
		catch (websocketpp::exception const& e) {
			XPLMDebugString(e.what());
			return 0;
		}
		XPLMCreateFlightLoop_t params = { sizeof(XPLMCreateFlightLoop_t), xplm_FlightLoop_Phase_AfterFlightModel, data_callback, nullptr };
		data_flight_loop_id = XPLMCreateFlightLoop(&params);
		if (data_flight_loop_id == nullptr)
		{
			XPLMDebugString("Cannot create flight loop. Exiting Ditto.\n");
			return 0;
		}
		else {
			XPLMScheduleFlightLoop(data_flight_loop_id, -1, true);
		}
	}
	XPLMDebugString("Enabling Ditto.\n");
	return 1;
}

PLUGIN_API void XPluginReceiveMessage(XPLMPluginID inFrom, int inMsg, void* inParam) { }

float data_callback(float inElapsedSinceLastCall,
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

float retry_callback(float inElapsedSinceLastCall,
	float inElapsedTimeSinceLastFlightLoop,
	int inCounter,
	void* inRefcon)
{
	if (new_data.get_not_found_list_size() != 0) {
		new_data.retry_dataref();
		return -1.0;
	}
	else {
		return 0;
	}
}