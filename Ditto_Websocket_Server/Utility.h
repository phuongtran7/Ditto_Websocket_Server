#pragma once
#include "XPLMProcessing.h"
#include "XPLMPlugin.h"
#include "XPLMDataAccess.h"
#include "XPLMUtilities.h"
#include "XPLMDataAccess.h"
#include <string>

// Some commn methods here

static std::string get_plugin_path() {
	const auto my_id = XPLMFindPluginBySignature("phuong.x-plane.ditto.websocket");;
	char buffer[1024]{};
	XPLMGetPluginInfo(my_id, nullptr, buffer, nullptr, nullptr);
	auto return_string = std::string(buffer);
	auto pos = return_string.find(R"(64\win.xpl)");
	if (pos != std::string::npos)
	{
		// XPLMGetPluginInfo return absolute path to win.xpl file so it need to be trimmed off the string
		return_string.erase(pos, return_string.length()); // Trim "64\win.xpl"
	}
	return return_string;
}