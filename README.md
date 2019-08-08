# Ditto Websocket Server
<h4 align="center">An X-Plane plugin that can add/remove/swap datarefs and endpoints on-the-fly.</h4>

Ditto is an X-Plane plugin that allows the user to pause the simulator in a specific scenario to add/remove or swap the datarefs that Ditto is sending the values out. This version of Ditto will create a small websocket server that the clients can connect to and receive the data. The data Ditto send out to the connected clients is a flatbuffers message with nested flexbuffers, represent the data, and an interger, represent the size of the nested flexbuffers.

Ditto uses <a href="https://google.github.io/flatbuffers/flexbuffers.html">Flexbuffers</a>, <a href="https://github.com/skystrife/cpptoml">cpptoml</a>, <a href="https://developer.x-plane.com/sdk/">X-Plane SDK</a> and <a href="https://github.com/zaphoyd/websocketpp">websocketpp</a>.

## Installation
### Windows
If you don't want to compile the plugin by yourself, you can head over the <a href="https://github.com/phuongtran7/Ditto_Websocket_Server/releases">releases</a> tab a get a pre-compiled version.

1. Install Flatbuffers, X-Plane SDK, cpptoml and Boost Asio with Microsoft's <a href="https://github.com/Microsoft/vcpkg">vcpkg</a>.
    * `vcpkg install flatbuffers`
    * `vcpkg install x-plane`
    * `vcpkg install websocketpp`
    * `vcpkg install cpptoml`
2. Clone the project: `git clone https://github.com/phuongtran7/Ditto_Websocket_Server`.
3. Build the project.

## Usage
### Start up
1. Copy the compiled Ditto into aircraft plugin folder in X-Plane. For example, `X_Plane root/Aircraft/Laminar Research/Boeing B737-800/plugins/`.
2. Copy `Datarefs.toml` file into Ditto folder. For example, `X_Plane root/Aircraft/Laminar Research/Boeing B737-800/plugins/Ditto/`. 
3. Define port number that Ditto will listen on. If `port` is not defined, Ditto will use default `1234` port.
4. Define all the datarefs that the plugin should send the value out. Ditto has the ability to retry finding the dataref if that dataref is created by another plugin that loaded after Ditto. However, looking for dataref is a rather exenpsive task, so Ditto only retrying after every 5 seconds. One way to work around that is making Ditto load last by renaming the Ditto plugin folder, for example `zDitto` and copy it into aircraft folder. This way Ditto will be loaded last, after other plugins finish publishing datarefs.
5. Start X-Plane.

### Modifying datarefs/endpoints
1. Disable Ditto by using Plugin Manager in X-Plane.
2. (Optional) Pause X-Plane.
3. Modify `Datarefs.toml`.
4. Re-enable Ditto and unpause X-Plane if necessary.

## Limitations
1. Even though Ditto supports multiple clients connecting at the same time, there is currently no way to specify which client should receive which values. All clients will be receiving the same data that is output by Ditto.

## Code samples
1. Example `Datarefs.toml` content:
```
# Setting port number
port = 8800

# Getting a float dataref
[[Data]] # Each dataref is an Data table
name = "airspeed" # User specify name of the dataref, which will be used to access data later
string = "sim/flightmodel/position/indicated_airspeed" # Dataref
type = "float" # Type of the dataref. Can be either "int", "float" or "char"

# Getting a float array dataref
[[Data]]
name = "fuel_flow"
string = "sim/cockpit2/engine/indicators/fuel_flow_kg_sec"
type = "float"
# If "start_index" and "num_value" present in the table, that dataref will be treated as an array of value
start_index = 0 # The start index of the array that Ditto should start reading from
num_value = 2 # Number of value after the start_index Ditto should read.

# Getting a string dataref
[[Data]]
name = "legs"
string = "laminar/B738/fms/legs"
type = "char"
start_index = 0
num_value = 512
```

2. Receiving data using `websocketpp`
```
#include <websocketpp/config/asio_no_tls_client.hpp>
#include <websocketpp/client.hpp>
#include "flatbuffers/flatbuffers.h"
#include "Schema_generated.h"
#include "spdlog/spdlog.h"

typedef websocketpp::client<websocketpp::config::asio_client> client;
typedef websocketpp::config::asio_client::message_type::ptr message_ptr;

void on_message(client* c, websocketpp::connection_hdl hdl, message_ptr msg) {
	auto payload = msg->get_payload();
	
	flatbuffers::Verifier verifier((uint8_t*)(payload.data()), payload.size());

	if (Ditto::VerifyDataBuffer(verifier)) {
		auto flat_root = Ditto::GetData(payload.data());

		const auto data = flat_root->buffer_flexbuffer_root().AsMap();

		spdlog::info("Airpseed: {:03.5f}", data["airspeed"].AsFloat());

		auto ff = data["fuel_flow"].AsVector();
		if (!ff.IsTheEmptyVector()) {
			spdlog::info("Fuel flow 0: {}", ff[0].AsFloat());
			spdlog::info("Fuel flow 1: {}", ff[1].AsFloat());
		}

		if (!data["legs"].AsString().IsTheEmptyString()) {
			spdlog::info("Legs: {}", data["legs"].AsString().c_str());
		}

		spdlog::info("-----------------");
	}
}
```