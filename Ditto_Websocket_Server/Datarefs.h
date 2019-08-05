#pragma once
#include <cpptoml.h>
#include <vector>
#include <optional>
#include "XPLMDataAccess.h"
#include "XPLMUtilities.h"
#include "flatbuffers/flatbuffers.h"
#include "flatbuffers/flexbuffers.h"
#include "Schema_generated.h"
#include "XPLMProcessing.h"
#include "XPLMPlugin.h"

class dataref {
private:
	struct dataref_info {
		std::string name{};
		XPLMDataRef dataref{};
		std::string type{};
		std::optional<int> start_index{};
		std::optional<int> num_value{}; // Number of values in the array to get starts at start_index
	};

	std::vector<dataref_info> dataref_list_;
	std::vector<dataref_info> get_list();
	void get_data_list();
	int get_value_int(XPLMDataRef in_dataref);
	float get_value_float(XPLMDataRef in_dataref);
	double get_value_double(XPLMDataRef in_dataref);
	void set_plugin_path();
	std::vector<int> get_value_int_array(XPLMDataRef in_dataref, int start_index, int end_index);
	std::vector<float> get_value_float_array(XPLMDataRef in_dataref, int start_index, int end_index);
	std::vector<char> get_value_char_array(XPLMDataRef in_dataref, int start_index, int end_index);
	std::vector<uint8_t> get_flexbuffers_data();
	size_t get_flexbuffers_size();
	std::string plugin_path_{};
	flexbuffers::Builder flexbuffers_builder_;
	flatbuffers::FlatBufferBuilder flatbuffers_builder_;
	bool status_{};
public:
	uint8_t* get_serialized_data();
	size_t get_serialized_size();
	void set_status(bool in_status);
	bool get_status();
	void empty_list();
	void reset_builder();
	void init();
};
